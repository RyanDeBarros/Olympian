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
			: degree(capacity.degree), polygon_index_count(capacity.polygon_index_count), ebo(vao, capacity.indices), transform_ssbo(capacity.primitives), position_vbo(capacity.vertices), color_vbo(capacity.vertices),
			vertex_free_space({ 0, capacity.primitives }), projection_bounds(projection_bounds)
		{
			shader = shaders::polygon_batch;
			projection_location = shaders::location(shader, "uProjection");
			degree_location = shaders::location(shader, "uDegree");

			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, position_vbo.buf.get_buffer());
			VertexAttribute<>{ 0, 2 }.setup();
			glBindBuffer(GL_ARRAY_BUFFER, color_vbo.buf.get_buffer());
			VertexAttribute<>{ 1, 4 }.setup();
			glBindVertexArray(0);
		}

		void PolygonBatch::render() const
		{
			position_vbo.pre_draw();
			color_vbo.pre_draw();
			transform_ssbo.pre_draw();

			glBindVertexArray(vao);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));
			glUniform1ui(degree_location, degree);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, transform_ssbo.buf.get_buffer());
			ebo.render_elements(GL_TRIANGLES);
			position_vbo.post_draw();
			color_vbo.post_draw();
			transform_ssbo.post_draw();
		}

		void PolygonBatch::set_primitive_points(Index pos, const glm::vec2* points, Index count)
		{
			Index vertices_offset = pos * degree;
			count = std::min(degree, count);
			for (Index v = 0; v < count; ++v)
				position_vbo.set(vertices_offset + v) = points[v];
		}

		void PolygonBatch::set_primitive_colors(Index pos, const glm::vec4* colors, Index count)
		{
			Index vertices_offset = pos * degree;
			count = std::min(degree, count);
			if (count == 1)
			{
				for (Index v = 0; v < degree; ++v)
					color_vbo.set(vertices_offset + v) = colors[0];
			}
			else
			{
				for (Index v = 0; v < count; ++v)
					color_vbo.set(vertices_offset + v) = colors[v];
			}
		}

		void PolygonBatch::set_primitive_transform(Index pos, const glm::mat3& transform)
		{
			if (pos >= transform_ssbo.buf.get_size())
				transform_ssbo.grow();
			transform_ssbo.set(pos) = transform;
		}

		void PolygonBatch::set_polygon_primitive(Index vertex_pos, const math::Polygon2D& polygon, const glm::mat3& transform)
		{
			set_primitive_points(vertex_pos, polygon.points.data(), (Index)polygon.points.size());
			set_primitive_colors(vertex_pos, polygon.colors.data(), (Index)polygon.colors.size());
			set_primitive_transform(vertex_pos, transform);
		}

		void PolygonBatch::set_polygon_transform(Index id, const glm::mat3& transform)
		{
			Range<Index> range = get_vertex_range(id);
			for (Index i = 0; i < range.length; ++i)
				set_primitive_transform(range.initial + i, transform);
		}

		PolygonBatch::PolygonID PolygonBatch::generate_id(const math::Polygon2DComposite& composite, Index min_range, Index max_range)
		{
			auto dup = dupl(composite);
			return generate_id(dup, min_range, max_range);
		}

		PolygonBatch::PolygonID PolygonBatch::generate_id(math::Polygon2DComposite& composite, Index min_range, Index max_range)
		{
			PolygonID id = id_generator.generate();
			math::split_polygon_composite(composite, degree);
			auto it = polygon_indexer.find(id.get());
			if (it == polygon_indexer.end())
			{
				Index range = std::max(min_range, (Index)composite.size());
				if (max_range > 0)
				{
					OLY_ASSERT(range <= max_range);
				}
				Range<Index> vertex_range, index_range;
				OLY_ASSERT(vertex_free_space.next_free(range, vertex_range));
				vertex_free_space.reserve(vertex_range);
				polygon_indexer[id.get()] = vertex_range;
				id_order[index_range.initial] = id.get();
			}
			return id;
		}

		void PolygonBatch::terminate_id(Index id)
		{
			auto it = polygon_indexer.find(id);
			if (it != polygon_indexer.end())
			{
				vertex_free_space.release(it->second);
				polygon_indexer.erase(it);
			}
		}

		void PolygonBatch::resize_range(PolygonID& id, const math::Polygon2DComposite& composite, Index min_range, Index max_range)
		{
			auto dup = dupl(composite);
			resize_range(id, dup, min_range, max_range);
		}

		void PolygonBatch::resize_range(PolygonID& id, math::Polygon2DComposite& composite, Index min_range, Index max_range)
		{
			terminate_id(id.get());
			id = generate_id(composite, min_range, max_range);
		}

		bool PolygonBatch::is_valid_id(Index id) const
		{
			return polygon_indexer.count(id);
		}

		Range<PolygonBatch::Index> PolygonBatch::get_vertex_range(Index id) const
		{
			if (!is_valid_id(id))
				throw Error(ErrorCode::OTHER, "Invalid Index");
			return polygon_indexer.find(id)->second;
		}

		void PolygonBatch::set_polygon(Index id, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform)
		{
			set_polygon(id, math::Polygon2DComposite{ polygon }, transform);
		}

		void PolygonBatch::set_polygon(Index id, math::TriangulatedPolygon2D&& polygon, const glm::mat3& transform)
		{
			set_polygon(id, math::Polygon2DComposite{ std::move(polygon) }, transform);
		}
		
		void PolygonBatch::set_polygon(Index id, const math::Polygon2DComposite& composite, const glm::mat3& transform)
		{
			auto dup = dupl(composite);
			set_polygon(id, dup, transform);
		}
		
		void PolygonBatch::set_polygon(Index id, math::Polygon2DComposite& composite, const glm::mat3& transform)
		{
			OLY_ASSERT(is_valid_id(id));
			auto vertex_range = get_vertex_range(id);
			math::split_polygon_composite(composite, degree);
			OLY_ASSERT(composite.size() <= vertex_range.length);
			Index length = 0;
			for (math::TriangulatedPolygon2D& poly : composite)
			{
				set_polygon_primitive(vertex_range.initial + length, std::move(poly.polygon), transform);
				++length;
			}
		}

		void PolygonBatch::set_ngon(Index id, const math::NGonBase& ngon, const glm::mat3& transform)
		{
			set_polygon(id, create_ngon(ngon), transform);
		}

		void PolygonBatch::set_bordered_ngon(Index id, const math::NGonBase& ngon, const glm::mat3& transform)
		{
			set_polygon(id, create_bordered_ngon(ngon), transform);
		}

		math::Polygon2DComposite PolygonBatch::create_ngon(const math::NGonBase& ngon) const
		{
			return ngon.composite(degree);
		}
		
		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(const math::NGonBase& ngon) const
		{
			return ngon.bordered_composite(degree);
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

			PolygonBatch::Index vertex_initial = _batch->get_vertex_range(id.get()).initial;
			auto comp = calc_composite(); // TODO use vector<Triangulation> instead
			math::split_polygon_composite(comp, _batch->degree);
			PolygonBatch::Index c = 0;
			for (const auto& tp : comp)
			{
				const auto& faces = tp.triangulation;
				for (size_t i = 0; i < faces.size() && i < _batch->polygon_index_count; ++i)
				{
					_batch->ebo.draw_primitive()[0] = faces[i][0] + (vertex_initial + c) * _batch->degree;
					_batch->ebo.draw_primitive()[0] = faces[i][1] + (vertex_initial + c) * _batch->degree;
					_batch->ebo.draw_primitive()[0] = faces[i][2] + (vertex_initial + c) * _batch->degree;
				}
				++c;
			}
		}

		void Polygonal::init(PolygonBatch::Index min_range, PolygonBatch::Index max_range)
		{
			OLY_ASSERT(!initialized());
			auto composite = calc_composite();
			id = _batch->generate_id(composite, min_range, max_range);
			transformer.pre_get();
			_batch->set_polygon(id.get(), std::move(composite), transformer.global());
		}

		void Polygonal::resize(PolygonBatch::Index min_range, PolygonBatch::Index max_range)
		{
			OLY_ASSERT(initialized());
			auto composite = calc_composite();
			_batch->resize_range(id, composite, min_range, max_range);
			transformer.pre_get();
			_batch->set_polygon(id.get(), std::move(composite), transformer.global());
		}

		void Polygonal::send_polygon() const
		{
			auto composite = calc_composite();
			auto vertex_range = _batch->get_vertex_range(id.get());
			for (PolygonBatch::Index i = 0; i < vertex_range.length; ++i)
			{
				_batch->set_primitive_points(vertex_range.initial + i, composite[i].polygon.points.data(), (PolygonBatch::Index)composite[i].polygon.points.size());
				_batch->set_primitive_colors(vertex_range.initial + i, composite[i].polygon.colors.data(), (PolygonBatch::Index)composite[i].polygon.colors.size());
			}
		}

		math::Polygon2DComposite Polygon::calc_composite() const
		{
			return { math::TriangulatedPolygon2D{ polygon, math::triangulate(polygon.points) } };
		}
		
		math::Polygon2DComposite Composite::calc_composite() const
		{
			return composite;
		}

		math::Polygon2DComposite NGon::calc_composite() const
		{
			// TODO cache composite
			return bordered ? batch().create_bordered_ngon(base) : batch().create_ngon(base);
		}
	}
}
