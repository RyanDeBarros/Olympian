#include "Polygons.h"

#include <glm/gtc/type_ptr.hpp>

#include "Resources.h"
#include "util/General.h"

namespace oly
{
	namespace batch
	{
		PolygonBatch::PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: capacity(capacity), ebo(capacity.indices), transform_ssbo(capacity.polygons), polygon_vbo(capacity.vertices, capacity.vertices), free_space_tracker({ 0, capacity.polygons })
		{
			shader = shaders::polygon_batch;
			glUseProgram(shader);
			projection_location = glGetUniformLocation(shader, "uProjection");
			degree_location = glGetUniformLocation(shader, "uDegree");

			glBindVertexArray(vao);
			polygon_vbo.vbo().init_layout(oly::rendering::VertexAttribute<>{ 0, 2 }, oly::rendering::VertexAttribute<>{ 1, 4 });
			ebo.init();

			set_projection(projection_bounds);

			glBindVertexArray(0);
		}

		void PolygonBatch::draw() const
		{
			glUseProgram(shader);
			glUniform1ui(degree_location, capacity.degree);
			glBindVertexArray(vao);
			transform_ssbo.bind_base(0);
			ebo.draw(GL_TRIANGLES, GL_UNSIGNED_SHORT);
		}

		void PolygonBatch::get_primitive_draw_spec(PrimitivePos& first, PrimitivePos& count) const
		{
			ebo.get_draw_spec(first, count);
			first /= capacity.polygon_index_count;
			count /= capacity.polygon_index_count;
		}

		void PolygonBatch::set_primitive_draw_spec(PrimitivePos first, PrimitivePos count)
		{
			ebo.set_draw_spec(first * capacity.polygon_index_count, count * capacity.polygon_index_count);
		}

		void PolygonBatch::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void PolygonBatch::set_primitive_points(PrimitivePos pos, const glm::vec2* points, GLushort count)
		{
			assert(pos < capacity.polygons);
			GLushort vertices_offset = pos * capacity.degree;
			count = std::min(capacity.degree, count);
			for (GLushort v = 0; v < count; ++v)
				polygon_vbo.vbo().vector<PolygonAttribute::POSITION>()[vertices_offset + v] = points[v];
			for (GLushort v = 0; v < capacity.degree; ++v)
				polygon_vbo.lazy_send<PolygonAttribute::POSITION>(vertices_offset + v);
		}

		void PolygonBatch::set_primitive_colors(PrimitivePos pos, const glm::vec4* colors, GLushort count)
		{
			assert(pos < capacity.polygons);
			GLushort vertices_offset = pos * capacity.degree;
			count = std::min(capacity.degree, count);
			if (count == 1)
			{
				for (GLushort v = 0; v < capacity.degree; ++v)
					polygon_vbo.vbo().vector<PolygonAttribute::COLOR>()[vertices_offset + v] = colors[0];
			}
			else
			{
				for (GLushort v = 0; v < count; ++v)
					polygon_vbo.vbo().vector<PolygonAttribute::COLOR>()[vertices_offset + v] = colors[v];
			}
			for (GLushort v = 0; v < capacity.degree; ++v)
				polygon_vbo.lazy_send<PolygonAttribute::COLOR>(vertices_offset + v);
		}

		void PolygonBatch::set_primitive_transform(PrimitivePos pos, const glm::mat3& transform)
		{
			assert(pos < capacity.polygons);
			transform_ssbo.vector()[pos] = transform;
			transform_ssbo.lazy_send(pos);
		}

		void PolygonBatch::set_primitive_triangulation(PrimitivePos pos, const math::Triangulation& triangulation)
		{
			assert(pos < capacity.polygons);
			GLushort indices_offset = pos * capacity.polygon_index_count;
			GLushort vertex_index_offset = pos * capacity.degree;
			auto& indices = ebo.vector();
			for (size_t i = 0; i < triangulation.faces.size() && i < capacity.polygon_index_count; ++i)
			{
				indices[indices_offset + 3 * i + 0] = triangulation.faces[i][0] + vertex_index_offset;
				indices[indices_offset + 3 * i + 1] = triangulation.faces[i][1] + vertex_index_offset;
				indices[indices_offset + 3 * i + 2] = triangulation.faces[i][2] + vertex_index_offset;
			}
			for (size_t i = triangulation.faces.size() * 3; i < capacity.polygon_index_count; i += 3)
			{
				indices[indices_offset + i + 0] = 0;
				indices[indices_offset + i + 1] = 0;
				indices[indices_offset + i + 2] = 0;
			}
			for (GLushort i = 0; i < capacity.polygon_index_count; ++i)
				ebo.lazy_send(indices_offset + i);
		}

		void PolygonBatch::set_polygon_primitive(PrimitivePos pos, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform)
		{
			set_primitive_points(pos, polygon.polygon.points.data(), (GLushort)polygon.polygon.points.size());
			set_primitive_colors(pos, polygon.polygon.colors.data(), (GLushort)polygon.polygon.colors.size());
			set_primitive_transform(pos, transform);
			set_primitive_triangulation(pos, polygon.triangulation);
		}

		void PolygonBatch::disable_polygon_primitive(PrimitivePos pos)
		{
			set_primitive_triangulation(pos, {});
		}

		void PolygonBatch::set_polygon_transform(RangeID id, const glm::mat3& transform)
		{
			Range<GLushort> range = get_range(id);
			for (GLushort i = 0; i < range.diff; ++i)
				set_primitive_transform(range.initial + i, transform);
		}

		void PolygonBatch::disable_polygon(RangeID id)
		{
			auto it = polygon_indexer.find(id);
			if (it != polygon_indexer.end())
				for (GLushort i = 0; i < it->second.diff; ++i)
					disable_polygon_primitive(it->second.initial + i);
		}

		PolygonBatch::RangeID PolygonBatch::generate_id(const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range)
		{
			auto dup = dupl(composite);
			return generate_id(dup, min_range, max_range);
		}

		PolygonBatch::RangeID PolygonBatch::generate_id(math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range)
		{
			RangeID id = id_generator.gen();
			math::split_polygon_composite(composite, capacity.degree);
			auto it = polygon_indexer.find(id);
			if (it == polygon_indexer.end())
			{
				GLushort range = std::max(min_range, (GLushort)composite.size());
				if (max_range > 0)
				{
					assert(range <= max_range);
				}
				Range<GLushort> composite_range;
				assert(free_space_tracker.next_free(range, composite_range));
				free_space_tracker.reserve(composite_range);
				polygon_indexer[id] = composite_range;
			}
			return id;
		}

		void PolygonBatch::terminate_id(RangeID id)
		{
			auto it = polygon_indexer.find(id);
			if (it != polygon_indexer.end())
			{
				disable_polygon(id);
				free_space_tracker.release(it->second);
				polygon_indexer.erase(it);
				id_generator.yield(id);
			}
		}

		void PolygonBatch::resize_range(RangeID id, const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range)
		{
			auto dup = dupl(composite);
			resize_range(id, dup, min_range, max_range);
		}

		void PolygonBatch::resize_range(RangeID id, math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range)
		{
			terminate_id(id);
			assert(generate_id(composite, min_range, max_range) == id);
		}

		bool PolygonBatch::is_valid_id(RangeID id) const
		{
			return polygon_indexer.count(id);
		}

		Range<GLushort> PolygonBatch::get_range(RangeID id) const
		{
			return polygon_indexer.find(id)->second;
		}

		void PolygonBatch::set_polygon(RangeID id, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform)
		{
			set_polygon(id, math::split_polygon_composite(polygon, capacity.degree), transform);
		}

		void PolygonBatch::set_polygon(RangeID id, math::TriangulatedPolygon2D&& polygon, const glm::mat3& transform)
		{
			set_polygon(id, math::split_polygon_composite(std::move(polygon), capacity.degree), transform);
		}
		
		void PolygonBatch::set_polygon(RangeID id, const math::Polygon2DComposite& composite, const glm::mat3& transform)
		{
			auto dup = dupl(composite);
			set_polygon(id, dup, transform);
		}
		
		void PolygonBatch::set_polygon(RangeID id, math::Polygon2DComposite& composite, const glm::mat3& transform)
		{
			assert(is_valid_id(id));
			auto composite_range = get_range(id);
			math::split_polygon_composite(composite, capacity.degree);
			assert(composite.size() <= composite_range.diff);
			GLushort diff = 0;
			for (math::TriangulatedPolygon2D& poly : composite)
				set_polygon_primitive(composite_range.initial + diff++, std::move(poly), transform);
			while (diff < composite_range.diff)
				disable_polygon(composite_range.initial + diff++);
		}

		void PolygonBatch::set_ngon(RangeID id, const math::NGonBase& ngon, const glm::mat3& transform)
		{
			set_polygon(id, create_ngon(ngon), transform);
		}

		void PolygonBatch::set_bordered_ngon(RangeID id, const math::NGonBase& ngon, const glm::mat3& transform)
		{
			set_polygon(id, create_bordered_ngon(ngon), transform);
		}

		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const
		{
			return math::create_bordered_ngon(fill_color, border_color, border, border_pivot, points);
		}
		
		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const
		{
			return math::create_bordered_ngon(fill_color, border_color, border, border_pivot, std::move(points));
		}

		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(const std::vector<glm::vec4>& fill_colors, const std::vector<glm::vec4>& border_colors,
			float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const
		{
			return math::create_bordered_ngon(fill_colors, border_colors, border, border_pivot, points);
		}

		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(std::vector<glm::vec4>&& fill_colors, std::vector<glm::vec4>&& border_colors,
			float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const
		{
			return math::create_bordered_ngon(std::move(fill_colors), std::move(border_colors), border, border_pivot, std::move(points));
		}

		math::Polygon2DComposite PolygonBatch::create_ngon(const math::NGonBase& ngon) const
		{
			return ngon.composite(capacity.degree);
		}
		
		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(const math::NGonBase& ngon) const
		{
			return ngon.bordered_composite(capacity.degree);
		}

		void PolygonBatch::flush() const
		{
			for (renderable::Polygonal* poly : polygonal_renderables)
				poly->flush();
			polygon_vbo.flush();
			transform_ssbo.flush();
			ebo.flush();
		}
	}

	namespace renderable
	{
		Polygonal::Polygonal(batch::PolygonBatch* batch)
			: _batch(batch), _transformer(std::make_unique<Transformer2D>())
		{
			_batch->polygonal_renderables.insert(this);
		}

		Polygonal::Polygonal(batch::PolygonBatch* batch, const Transform2D& local)
			: _batch(batch), _transformer(std::make_unique<Transformer2D>(local))
		{
			_batch->polygonal_renderables.insert(this);
		}

		Polygonal::Polygonal(batch::PolygonBatch* batch, std::unique_ptr<Transformer2D>&& transformer)
			: _batch(batch), _transformer(std::move(transformer))
		{
			_batch->polygonal_renderables.insert(this);
		}

		Polygonal::Polygonal(Polygonal&& other) noexcept
			: _batch(other._batch), id(other.id), _transformer(std::move(other._transformer))
		{
			if (_batch)
				_batch->polygonal_renderables.insert(this);
			other.id = -1;
		}

		Polygonal::~Polygonal()
		{
			if (_batch)
			{
				_batch->polygonal_renderables.erase(this);
				if (id != -1)
					_batch->terminate_id(id);
			}
		}

		Polygonal& Polygonal::operator=(Polygonal&& other) noexcept
		{
			if (this != &other)
			{
				if (_batch)
					_batch->polygonal_renderables.erase(this);
				_batch = other._batch;
				if (_batch)
					_batch->polygonal_renderables.insert(this);
				id = other.id;
				other.id = -1;
				_transformer = std::move(other._transformer);
			}
			return *this;
		}

		void Polygonal::post_set() const
		{
			_transformer->post_set();
		}
		
		void Polygonal::pre_get() const
		{
			_transformer->pre_get();
		}
		
		void Polygonal::init(const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range)
		{
			assert(id == batch::PolygonBatch::RangeID(-1));
			id = _batch->generate_id(composite, min_range, max_range);
			_transformer->pre_get();
			_batch->set_polygon(id, composite, _transformer->global());
		}

		void Polygonal::init(math::Polygon2DComposite&& composite, GLushort min_range, GLushort max_range)
		{
			assert(id == batch::PolygonBatch::RangeID(-1));
			id = _batch->generate_id(composite, min_range, max_range);
			_transformer->pre_get();
			_batch->set_polygon(id, std::move(composite), _transformer->global());
		}

		void Polygonal::resize(const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range)
		{
			assert(this->id != batch::PolygonBatch::RangeID(-1));
			_batch->resize_range(id, composite, min_range, max_range);
			_transformer->pre_get();
			_batch->set_polygon(id, composite, _transformer->global());
		}

		void Polygonal::resize(math::Polygon2DComposite&& composite, GLushort min_range, GLushort max_range)
		{
			assert(this->id != batch::PolygonBatch::RangeID(-1));
			_batch->resize_range(id, composite, min_range, max_range);
			_transformer->pre_get();
			_batch->set_polygon(id, std::move(composite), _transformer->global());
		}

		void Polygonal::send_polygon(const math::Polygon2DComposite& composite) const
		{
			Range<GLushort> range = _batch->get_range(id);
			for (GLushort i = 0; i < range.diff; ++i)
			{
				_batch->set_primitive_points(range.initial + i, composite[i].polygon.points.data(), (GLushort)composite[i].polygon.points.size());
				_batch->set_primitive_colors(range.initial + i, composite[i].polygon.colors.data(), (GLushort)composite[i].polygon.colors.size());
				_batch->set_primitive_triangulation(range.initial + i, composite[i].triangulation);
			}
		}

		void Polygonal::flush() const
		{
			if (_transformer->flush())
			{
				_transformer->pre_get();
				_batch->set_polygon_transform(id, _transformer->global());
			}
		}
		
		void Polygon::init(GLushort min_range, GLushort max_range)
		{
			Polygonal::init(math::split_polygon_composite(polygon, batch()->max_degree()), min_range, max_range);
		}

		void Polygon::resize(GLushort min_range, GLushort max_range)
		{
			Polygonal::resize(math::split_polygon_composite(polygon, batch()->max_degree()), min_range, max_range);
		}
		
		void Polygon::send_polygon() const
		{
			Polygonal::send_polygon(math::split_polygon_composite(polygon, batch()->max_degree()));
		}
		
		void Composite::init(GLushort min_range, GLushort max_range)
		{
			auto dup = dupl(composite);
			Polygonal::init(dup, min_range, max_range);
		}

		void Composite::resize(GLushort min_range, GLushort max_range)
		{
			auto dup = dupl(composite);
			Polygonal::resize(dup, min_range, max_range);
		}

		void Composite::send_polygon() const
		{
			math::Polygon2DComposite dup = dupl(composite);
			math::split_polygon_composite(dup, batch()->max_degree());
			Polygonal::send_polygon(dup);
		}

		void NGon::init(GLushort min_range, GLushort max_range)
		{
			Polygonal::init(composite(), min_range, max_range);
		}

		void NGon::resize(GLushort min_range, GLushort max_range)
		{
			Polygonal::resize(composite(), min_range, max_range);
		}

		void NGon::send_polygon() const
		{
			Polygonal::send_polygon(composite());
		}

		math::Polygon2DComposite NGon::composite() const
		{
			return bordered ? batch()->create_bordered_ngon(base) : batch()->create_ngon(base);
		}
	}
}
