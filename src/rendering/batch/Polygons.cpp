#include "Polygons.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include "../Resources.h"
#include "util/General.h"
#include "math/DataStructures.h"

namespace oly
{
	namespace rendering
	{
		PolygonBatch::PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ebo(vao, capacity.indices), bo_block({ capacity.vertices, capacity.vertices, capacity.vertices, capacity.indices }), vertex_free_space({ 0, capacity.primitives }), projection_bounds(projection_bounds)
		{
			shader = shaders::polygon_batch;
			projection_location = shaders::location(shader, "uProjection");

			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, bo_block.buf.get_buffer<POSITION>());
			VertexAttribute<>{ 0, 2 }.setup();
			glBindBuffer(GL_ARRAY_BUFFER, bo_block.buf.get_buffer<COLOR>());
			VertexAttribute<>{ 1, 4 }.setup();
			glBindBuffer(GL_ARRAY_BUFFER, bo_block.buf.get_buffer<INDEX>());
			VertexAttribute<VertexAttributeType::INT>{ 2, 1 }.setup();
			glBindVertexArray(0);
		}

		void PolygonBatch::render() const
		{
			bo_block.pre_draw_all();
			glBindVertexArray(vao);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bo_block.buf.get_buffer<TRANSFORM>());
			ebo.render_elements(GL_TRIANGLES);
			bo_block.post_draw_all();
		}

		void PolygonBatch::set_primitive_points(Range<Index> range, const glm::vec2* points, Index count)
		{
			count = std::min(range.length, count);
			for (Index v = 0; v < count; ++v)
				bo_block.set<POSITION>(range.initial + v) = points[v];
		}

		void PolygonBatch::set_primitive_colors(Range<Index> range, const glm::vec4* colors, Index count)
		{
			count = std::min(range.length, count);
			if (count == 1)
			{
				for (Index v = 0; v < range.length; ++v)
					bo_block.set<COLOR>(range.initial + v) = colors[0];
			}
			else
			{
				for (Index v = 0; v < count; ++v)
					bo_block.set<COLOR>(range.initial + v) = colors[v];
			}
		}

		void PolygonBatch::set_polygon(Index id, const math::Polygon2D& polygon)
		{
			OLY_ASSERT(is_valid_id(id));
			auto vertex_range = get_vertex_range(id);
			set_primitive_points(vertex_range, polygon.points.data(), (Index)polygon.points.size());
			set_primitive_colors(vertex_range, polygon.colors.data(), (Index)polygon.colors.size());
		}

		void PolygonBatch::set_polygon(Index id, const std::vector<math::Polygon2D>& polygons)
		{
			OLY_ASSERT(is_valid_id(id));
			auto vertex_range = get_vertex_range(id);
			Index offset = 0;
			for (const math::Polygon2D& poly : polygons)
			{
				Range<Index> poly_range;
				poly_range.initial = vertex_range.initial + offset;
				if (poly_range.initial >= vertex_range.end())
					return;
				poly_range.length = std::min((Index)poly.points.size(), vertex_range.end() - poly_range.initial);
				set_primitive_points(poly_range, poly.points.data(), (Index)poly.points.size());
				set_primitive_colors(poly_range, poly.colors.data(), (Index)poly.colors.size());
				offset += (Index)poly.points.size();
			}
		}

		void PolygonBatch::set_polygon(Index id, const math::Polygon2DComposite& composite)
		{
			OLY_ASSERT(is_valid_id(id));
			auto vertex_range = get_vertex_range(id);
			Index offset = 0;
			for (const math::TriangulatedPolygon2D& poly : composite)
			{
				Range<Index> poly_range;
				poly_range.initial = vertex_range.initial + offset;
				if (poly_range.initial >= vertex_range.end())
					return;
				poly_range.length = std::min((Index)poly.polygon.points.size(), vertex_range.end() - poly_range.initial);
				set_primitive_points(poly_range, poly.polygon.points.data(), (Index)poly.polygon.points.size());
				set_primitive_colors(poly_range, poly.polygon.colors.data(), (Index)poly.polygon.colors.size());
				offset += (Index)poly.polygon.points.size();
			}
		}

		void PolygonBatch::set_polygon_transform(Index id, const glm::mat3& transform)
		{
			if (id >= bo_block.buf.get_size<TRANSFORM>())
			{
				bo_block.grow<TRANSFORM>();
				vertex_free_space.extend_rightward(bo_block.buf.get_size<TRANSFORM>());
			}
			bo_block.buf.at<TRANSFORM>(id) = transform;
			bo_block.flag<TRANSFORM>(id);
		}

		PolygonBatch::PolygonID PolygonBatch::generate_id(Index vertices)
		{
			PolygonID id = id_generator.generate();
			auto it = polygon_indexer.find(id.get());
			if (it == polygon_indexer.end())
			{
				Range<Index> vertex_range, index_range;
				OLY_ASSERT(vertex_free_space.next_free(vertices, vertex_range));
				vertex_free_space.reserve(vertex_range);
				polygon_indexer[id.get()] = vertex_range;
				for (Index v = 0; v < vertex_range.length; ++v)
					bo_block.set<INDEX>(vertex_range.initial + v) = id.get();
			}
			return id;
		}

		void PolygonBatch::terminate_id(Index id)
		{
			if (id == (Index)-1)
				return;
			auto it = polygon_indexer.find(id);
			if (it != polygon_indexer.end())
			{
				vertex_free_space.release(it->second);
				polygon_indexer.erase(it);
			}
		}

		void PolygonBatch::resize_range(PolygonID& id, Index vertices)
		{
			if (is_valid_id(id.get()) && vertices == get_vertex_range(id.get()).length)
				return;
			terminate_id(id.get());
			id = generate_id(vertices);
		}

		Range<PolygonBatch::Index> PolygonBatch::get_vertex_range(Index id) const
		{
			if (!is_valid_id(id))
				throw Error(ErrorCode::OTHER, "Invalid Index");
			return polygon_indexer.find(id)->second;
		}

		bool PolygonBatch::is_valid_id(Index id) const
		{
			return polygon_indexer.count(id);
		}

		Polygonal::Polygonal(PolygonBatch* batch)
			: _batch(batch)
		{
		}

		Polygonal::Polygonal(Polygonal&& other) noexcept
			: _batch(other._batch), id(std::move(other.id)), transformer(std::move(other.transformer))
		{
			other._batch = nullptr;
		}

		Polygonal::~Polygonal()
		{
			if (_batch)
				_batch->terminate_id(id.get());
		}

		Polygonal& Polygonal::operator=(Polygonal&& other) noexcept
		{
			if (this != &other)
			{
				_batch = other._batch;
				other._batch = nullptr;
				id = std::move(other.id);
				transformer = std::move(other.transformer);
			}
			return *this;
		}

		void Polygonal::post_set() const
		{
			transformer.post_set();
		}
		
		void Polygonal::pre_get() const
		{
			transformer.pre_get();
		}
		
		void Polygonal::draw() const
		{
			if (transformer.flush())
			{
				transformer.pre_get();
				_batch->set_polygon_transform(id.get(), transformer.global());
			}
			draw_triangulation(_batch->get_vertex_range(id.get()).initial);
		}

		void Polygonal::init()
		{
			_batch->resize_range(id, num_vertices());
			transformer.pre_get();
			send_polygon();
		}

		void Polygonal::set_polygon(const math::Polygon2D& polygon) const
		{
			_batch->set_polygon(id.get(), polygon);
		}

		void Polygonal::set_polygon(const std::vector<math::Polygon2D>& polygons) const
		{
			_batch->set_polygon(id.get(), polygons);
		}

		void Polygonal::set_polygon(const math::Polygon2DComposite& composite) const
		{
			_batch->set_polygon(id.get(), composite);
		}

		GLuint& Polygonal::draw_index() const
		{
			return _batch->ebo.draw_primitive()[0];
		}

		void Polygon::send_polygon() const
		{
			cache = math::triangulate(polygon.points);
			set_polygon(polygon);
		}

		GLuint Polygon::num_vertices() const
		{
			return (GLuint)polygon.points.size();
		}

		void Polygon::draw_triangulation(GLuint initial_vertex) const
		{
			for (size_t i = 0; i < cache.size(); ++i)
			{
				draw_index() = cache[i][0] + initial_vertex;
				draw_index() = cache[i][1] + initial_vertex;
				draw_index() = cache[i][2] + initial_vertex;
			}
		}

		void Composite::send_polygon() const
		{
			set_polygon(composite);
		}

		GLuint Composite::num_vertices() const
		{
			GLuint vertices = 0;
			for (const auto& primitive : composite)
				vertices += (GLuint)primitive.polygon.points.size();
			return vertices;
		}

		void Composite::draw_triangulation(GLuint initial_vertex) const
		{
			GLuint offset = 0;
			for (const auto& tp : composite)
			{
				const auto& faces = tp.triangulation;
				for (size_t i = 0; i < faces.size(); ++i)
				{
					draw_index() = faces[i][0] + initial_vertex + offset;
					draw_index() = faces[i][1] + initial_vertex + offset;
					draw_index() = faces[i][2] + initial_vertex + offset;
				}
				offset += (GLuint)tp.polygon.points.size();
			}
		}

		void NGon::send_polygon() const
		{
			cache = bordered ? base.bordered_composite() : base.composite();
			set_polygon(cache);
		}

		GLuint NGon::num_vertices() const
		{
			GLuint vertices = 0;
			for (const auto& primitive : cache)
				vertices += (GLuint)primitive.polygon.points.size();
			return vertices;
		}

		void NGon::draw_triangulation(GLuint initial_vertex) const
		{
			GLuint offset = 0;
			for (const auto& tp : cache)
			{
				const auto& faces = tp.triangulation;
				for (size_t i = 0; i < faces.size(); ++i)
				{
					draw_index() = faces[i][0] + initial_vertex + offset;
					draw_index() = faces[i][1] + initial_vertex + offset;
					draw_index() = faces[i][2] + initial_vertex + offset;
				}
				offset += (GLuint)tp.polygon.points.size();
			}
		}
	}
}
