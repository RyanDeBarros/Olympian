#include "Shapes.h"

#include <glm/gtc/type_ptr.hpp>

#include "Resources.h"
#include "util/General.h"

namespace oly
{
	namespace batch
	{
		PolygonBatch::PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: capacity(capacity), ebo(capacity.indices), transform_ssbo(capacity.polygons), polygon_vbo(capacity.vertices, capacity.vertices)
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

		void PolygonBatch::set_range_transform(Range<GLushort> range, const glm::mat3& transform)
		{
			for (GLushort i = 0; i < range.diff; ++i)
				set_primitive_transform(range.initial + i, transform);
		}

		void PolygonBatch::disable_polygon_range(Range<GLushort> range)
		{
			for (GLushort i = 0; i < range.diff; ++i)
				disable_polygon(range.initial + i);
		}

		void PolygonBatch::disable_polygon(PolygonPos pos)
		{
			if (polygon_indexer.exists(pos))
				for (GLushort diff = 0; diff < polygon_indexer.get_range(pos); ++diff)
					disable_polygon(polygon_indexer.get_pos(pos) + diff);
		}

		Range<GLushort> PolygonBatch::set_polygon(PolygonPos pos, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform, GLushort min_range, GLushort max_range)
		{
			return set_polygon(pos, math::split_polygon_composite(polygon, capacity.degree), transform, min_range, max_range);
		}

		Range<GLushort> PolygonBatch::set_polygon(PolygonPos pos, math::TriangulatedPolygon2D&& polygon, const glm::mat3& transform, GLushort min_range, GLushort max_range)
		{
			return set_polygon(pos, math::split_polygon_composite(std::move(polygon), capacity.degree), transform, min_range, max_range);
		}
		
		Range<GLushort> PolygonBatch::set_polygon(PolygonPos pos, const math::Polygon2DComposite& composite, const glm::mat3& transform, GLushort min_range, GLushort max_range)
		{
			return set_polygon(pos, dupl(composite), transform, min_range, max_range);
		}
		
		Range<GLushort> PolygonBatch::set_polygon(PolygonPos pos, math::Polygon2DComposite&& composite, const glm::mat3& transform, GLushort min_range, GLushort max_range)
		{
			math::split_polygon_composite(composite, capacity.degree);
			GLushort start = polygon_indexer.exists(pos) ? polygon_indexer.get_pos(pos) : polygon_indexer.next_pos();
			GLushort composite_range;
			if (polygon_indexer.exists(pos))
			{
				composite_range = polygon_indexer.get_range(pos);
				assert(composite.size() <= composite_range);
			}
			else if (max_range == 0)
				composite_range = std::max(min_range, (GLushort)composite.size());
			else
			{
				assert(composite.size() <= max_range && min_range <= max_range);
				composite_range = std::max(min_range, max_range);
			}
			GLushort diff = 0;
			for (math::TriangulatedPolygon2D& poly : composite)
				set_polygon_primitive(start + diff++, std::move(poly), transform);
			while (diff < composite_range)
				disable_polygon(start + diff++);
			if (!polygon_indexer.exists(pos))
				polygon_indexer.register_composite(start, composite_range);

			return { start, composite_range };
		}

		Range<GLushort> PolygonBatch::set_ngon(PolygonPos pos, const math::NGonBase& ngon, const glm::mat3& transform, GLushort min_range, GLushort max_range)
		{
			return set_polygon(pos, create_ngon(ngon), transform, min_range, max_range);
		}

		Range<GLushort> PolygonBatch::set_bordered_ngon(PolygonPos pos, const math::NGonBase& ngon, const glm::mat3& transform, GLushort min_range, GLushort max_range)
		{
			return set_polygon(pos, create_bordered_ngon(ngon), transform, min_range, max_range);
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
			: _batch(other._batch), range(other.range), _transformer(std::move(other._transformer))
		{
			if (_batch)
				_batch->polygonal_renderables.insert(this);
		}

		Polygonal::~Polygonal()
		{
			if (_batch)
				_batch->polygonal_renderables.erase(this);
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
				range = other.range;
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
		
		void Polygonal::init(batch::PolygonBatch::PolygonPos pos, const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range)
		{
			assert(range.diff == 0);
			_transformer->pre_get();
			range = _batch->set_polygon(pos, composite, _transformer->global(), min_range, max_range);
		}

		void Polygonal::init(batch::PolygonBatch::PolygonPos pos, math::Polygon2DComposite&& composite, GLushort min_range, GLushort max_range)
		{
			assert(range.diff == 0);
			_transformer->pre_get();
			range = _batch->set_polygon(pos, std::move(composite), _transformer->global(), min_range, max_range);
		}

		void Polygonal::send_polygon(const math::Polygon2DComposite& composite) const
		{
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
				_batch->set_range_transform(range, _transformer->global());
			}
		}
		
		void Polygon::init(batch::PolygonBatch::PolygonPos pos, GLushort min_range, GLushort max_range)
		{
			Polygonal::init(pos, math::split_polygon_composite(polygon, _batch->max_degree()), min_range, max_range);
		}
		
		void Polygon::send_polygon() const
		{
			Polygonal::send_polygon(math::split_polygon_composite(polygon, _batch->max_degree()));
		}
		
		void Composite::init(batch::PolygonBatch::PolygonPos pos, GLushort min_range, GLushort max_range)
		{
			Polygonal::init(pos, composite, min_range, max_range);
		}

		void Composite::send_polygon() const
		{
			math::Polygon2DComposite dup = dupl(composite);
			math::split_polygon_composite(dup, _batch->max_degree());
			Polygonal::send_polygon(dup);
		}

		void NGon::init(batch::PolygonBatch::PolygonPos pos, bool gen_border, GLushort min_range, GLushort max_range)
		{
			bordered = gen_border;
			Polygonal::init(pos, bordered ? _batch->create_bordered_ngon(base) : _batch->create_ngon(base), min_range, max_range);
		}

		void NGon::send_polygon() const
		{
			Polygonal::send_polygon(bordered ? _batch->create_bordered_ngon(base) : _batch->create_ngon(base));
		}
	}
}
