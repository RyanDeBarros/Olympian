#include "Polygons.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include "Resources.h"
#include "util/General.h"
#include "math/DataStructures.h"

namespace oly
{
	namespace batch
	{
		PolygonBatch::PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: capacity(capacity), ebo(capacity.indices), transform_ssbo(capacity.primitives), polygon_vbo(capacity.vertices, capacity.vertices),
			vertex_free_space({ 0, capacity.primitives }), index_free_space({ 0, capacity.primitives })
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
			draw_specs.push_back({ 0, capacity.primitives });
		}

		void PolygonBatch::draw(size_t draw_spec)
		{
			glUseProgram(shader);
			glUniform1ui(degree_location, capacity.degree);
			glBindVertexArray(vao);
			transform_ssbo.bind_base(0);
			set_primitive_draw_spec(draw_specs[draw_spec].initial, draw_specs[draw_spec].length);
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
			OLY_ASSERT(pos < capacity.primitives);
			GLushort vertices_offset = pos * capacity.degree;
			count = std::min(capacity.degree, count);
			for (GLushort v = 0; v < count; ++v)
				polygon_vbo.vbo().vector<PolygonAttribute::POSITION>()[vertices_offset + v] = points[v];
			for (GLushort v = 0; v < capacity.degree; ++v)
				polygon_vbo.lazy_send<PolygonAttribute::POSITION>(vertices_offset + v);
		}

		void PolygonBatch::set_primitive_colors(PrimitivePos pos, const glm::vec4* colors, GLushort count)
		{
			OLY_ASSERT(pos < capacity.primitives);
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
			OLY_ASSERT(pos < capacity.primitives);
			transform_ssbo.vector()[pos] = transform;
			transform_ssbo.lazy_send(pos);
		}

		void PolygonBatch::set_primitive_triangulation(PrimitivePos vertex_pos, PrimitivePos index_pos, const math::Triangulation& triangulation)
		{
			OLY_ASSERT(vertex_pos < capacity.primitives && index_pos < capacity.primitives);
			GLushort indices_offset = index_pos * capacity.polygon_index_count;
			GLushort vertex_index_offset = vertex_pos * capacity.degree;
			auto& indices = ebo.vector();
			for (size_t i = 0; i < triangulation.size() && i < capacity.polygon_index_count; ++i)
			{
				indices[indices_offset + 3 * i + 0] = triangulation[i][0] + vertex_index_offset;
				indices[indices_offset + 3 * i + 1] = triangulation[i][1] + vertex_index_offset;
				indices[indices_offset + 3 * i + 2] = triangulation[i][2] + vertex_index_offset;
			}
			for (size_t i = triangulation.size() * 3; i < capacity.polygon_index_count; i += 3)
			{
				indices[indices_offset + i + 0] = 0;
				indices[indices_offset + i + 1] = 0;
				indices[indices_offset + i + 2] = 0;
			}
			for (GLushort i = 0; i < capacity.polygon_index_count; ++i)
				ebo.lazy_send(indices_offset + i);
		}

		void PolygonBatch::set_polygon_primitive(PrimitivePos vertex_pos, PrimitivePos index_pos, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform)
		{
			set_primitive_points(vertex_pos, polygon.polygon.points.data(), (GLushort)polygon.polygon.points.size());
			set_primitive_colors(vertex_pos, polygon.polygon.colors.data(), (GLushort)polygon.polygon.colors.size());
			set_primitive_transform(index_pos, transform);
			set_primitive_triangulation(vertex_pos, index_pos, polygon.triangulation);
		}

		void PolygonBatch::disable_polygon_primitive(PrimitivePos index_pos)
		{
			set_primitive_triangulation(0, index_pos, {});
		}

		void PolygonBatch::set_polygon_transform(RangeID id, const glm::mat3& transform)
		{
			Range<GLushort> range = get_vertex_range(id);
			for (GLushort i = 0; i < range.length; ++i)
				set_primitive_transform(range.initial + i, transform);
		}

		void PolygonBatch::disable_polygon(RangeID id)
		{
			auto it = polygon_indexer.find(id);
			if (it != polygon_indexer.end())
			{
				Range<PrimitivePos> range = it->second.index_range;
				for (GLushort i = 0; i < range.length; ++i)
					disable_polygon_primitive(range.initial + i);
			}
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
					OLY_ASSERT(range <= max_range);
				}
				Range<GLushort> vertex_range, index_range;
				OLY_ASSERT(vertex_free_space.next_free(range, vertex_range));
				OLY_ASSERT(index_free_space.next_free(range, index_range));
				vertex_free_space.reserve(vertex_range);
				index_free_space.reserve(index_range);
				polygon_indexer[id].vertex_range = vertex_range;
				polygon_indexer[id].index_range = index_range;
				id_order[index_range.initial] = id;
			}
			return id;
		}

		void PolygonBatch::terminate_id(RangeID id)
		{
			auto it = polygon_indexer.find(id);
			if (it != polygon_indexer.end())
			{
				disable_polygon(id);
				vertex_free_space.release(it->second.vertex_range);
				index_free_space.release(it->second.index_range);
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
			OLY_ASSERT(generate_id(composite, min_range, max_range) == id);
		}

		bool PolygonBatch::is_valid_id(RangeID id) const
		{
			return polygon_indexer.count(id);
		}

		bool PolygonBatch::get_next_draw_id(RangeID id, RangeID& next_id) const
		{
			auto it = polygon_indexer.find(id);
			if (it == polygon_indexer.end())
				return false;
			auto nit = id_order.find(it->second.index_range.initial);
			if (nit == id_order.end())
				return false;
			++nit;
			if (nit == id_order.end())
				return false;
			next_id = nit->second;
			return true;
		}

		bool PolygonBatch::get_prev_draw_id(RangeID id, RangeID& prev_id) const
		{
			auto it = polygon_indexer.find(id);
			if (it == polygon_indexer.end())
				return false;
			auto pit = id_order.find(it->second.index_range.initial);
			if (pit == id_order.end() || pit == id_order.begin())
				return false;
			--pit;
			prev_id = pit->second;
			return true;
		}

		Range<PolygonBatch::PrimitivePos> PolygonBatch::get_vertex_range(RangeID id) const
		{
			if (!is_valid_id(id))
				throw Error(ErrorCode::OTHER, "Invalid RangeID");
			return polygon_indexer.find(id)->second.vertex_range;
		}

		Range<PolygonBatch::PrimitivePos> PolygonBatch::get_index_range(RangeID id) const
		{
			if (!is_valid_id(id))
				throw Error(ErrorCode::OTHER, "Invalid RangeID");
			return polygon_indexer.find(id)->second.index_range;
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
			OLY_ASSERT(is_valid_id(id));
			auto vertex_range = get_vertex_range(id);
			auto index_range = get_index_range(id);
			math::split_polygon_composite(composite, capacity.degree);
			OLY_ASSERT(composite.size() <= vertex_range.length);
			GLushort length = 0;
			for (math::TriangulatedPolygon2D& poly : composite)
			{
				set_polygon_primitive(vertex_range.initial + length, index_range.initial + length, std::move(poly), transform);
				++length;
			}
			while (length < index_range.length)
				disable_polygon_primitive(index_range.initial + length++);
		}

		void PolygonBatch::set_ngon(RangeID id, const math::NGonBase& ngon, const glm::mat3& transform)
		{
			set_polygon(id, create_ngon(ngon), transform);
		}

		void PolygonBatch::set_bordered_ngon(RangeID id, const math::NGonBase& ngon, const glm::mat3& transform)
		{
			set_polygon(id, create_bordered_ngon(ngon), transform);
		}

		void PolygonBatch::swap_poly_order_with_next(RangeID id)
		{
			RangeID next_id;
			if (!get_next_draw_id(id, next_id))
				return;
			Range<PrimitivePos>& this_range = polygon_indexer.find(id)->second.index_range;
			Range<PrimitivePos>& next_range = polygon_indexer.find(next_id)->second.index_range;
			swap_adjacent_poly_orders(this_range, next_range);

			id_order.erase(this_range.initial);
			id_order.erase(next_range.initial);
			index_free_space.release(this_range);
			index_free_space.release(next_range);
			PrimitivePos next_initial = next_range.initial;
			next_range.initial = this_range.initial;
			this_range.initial = next_initial + next_range.length - this_range.length;
			id_order[this_range.initial] = id;
			id_order[next_range.initial] = next_id;
			index_free_space.reserve(this_range);
			index_free_space.reserve(next_range);
		}

		void PolygonBatch::swap_poly_order_with_prev(RangeID id)
		{
			RangeID prev_id;
			if (!get_prev_draw_id(id, prev_id))
				return;
			Range<PrimitivePos>& this_range = polygon_indexer.find(id)->second.index_range;
			Range<PrimitivePos>& prev_range = polygon_indexer.find(prev_id)->second.index_range;
			swap_adjacent_poly_orders(prev_range, this_range);

			id_order.erase(this_range.initial);
			id_order.erase(prev_range.initial);
			index_free_space.release(this_range);
			index_free_space.release(prev_range);
			PrimitivePos this_initial = this_range.initial;
			this_range.initial = prev_range.initial;
			prev_range.initial = this_initial + this_range.length - prev_range.length;
			id_order[this_range.initial] = id;
			id_order[prev_range.initial] = prev_id;
			index_free_space.reserve(this_range);
			index_free_space.reserve(prev_range);
		}

		void PolygonBatch::swap_adjacent_poly_orders(Range<PrimitivePos>& left, Range<PrimitivePos>& right)
		{
			swap_adjacent_subbuffers(ebo.vector().data(), left.initial * capacity.polygon_index_count, left.length * capacity.polygon_index_count,
				right.initial * capacity.polygon_index_count, right.length * capacity.polygon_index_count);
			PrimitivePos first = std::min(left.initial, right.initial);
			PrimitivePos last = std::max(left.end(), right.end());
			for (PrimitivePos i = first; i < last; ++i)
				for (GLushort j = 0; j < capacity.polygon_index_count; ++j)
					ebo.lazy_send(i * capacity.polygon_index_count + j);
		}

		void PolygonBatch::move_poly_order_after(RangeID id, RangeID after)
		{
			auto this_it = polygon_indexer.find(id);
			auto after_it = polygon_indexer.find(after);
			if (this_it == after_it || this_it == polygon_indexer.end() || after_it == polygon_indexer.end())
				return;

			auto this_order_it = id_order.find(this_it->second.index_range.initial);
			auto after_order_it = id_order.find(after_it->second.index_range.initial);
			if (after_it->second.index_range.initial > this_it->second.index_range.initial)
			{
				auto distance = std::distance(this_order_it, after_order_it);
				for (int i = 0; i < distance + 1; ++i)
					swap_poly_order_with_next(id);
			}
			else
			{
				auto distance = std::distance(after_order_it, this_order_it);
				for (int i = 0; i < distance - 1; ++i)
					swap_poly_order_with_prev(id);
			}
		}

		void PolygonBatch::move_poly_order_before(RangeID id, RangeID before)
		{
			auto this_it = polygon_indexer.find(id);
			auto before_it = polygon_indexer.find(before);
			if (this_it == before_it || this_it == polygon_indexer.end() || before_it == polygon_indexer.end())
				return;

			auto this_order_it = id_order.find(this_it->second.index_range.initial);
			auto before_order_it = id_order.find(before_it->second.index_range.initial);
			if (before_it->second.index_range.initial > this_it->second.index_range.initial)
			{
				auto distance = std::distance(this_order_it, before_order_it);
				for (int i = 0; i < distance - 1; ++i)
					swap_poly_order_with_next(id);
			}
			else
			{
				auto distance = std::distance(before_order_it, this_order_it);
				for (int i = 0; i < distance + 1; ++i)
					swap_poly_order_with_prev(id);
			}
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
			: _batch(batch)
		{
			_batch->polygonal_renderables.insert(this);
		}

		Polygonal::Polygonal(Polygonal&& other) noexcept
			: _batch(other._batch), id(other.id), transformer(std::move(other.transformer))
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
		
		void Polygonal::init(const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range)
		{
			OLY_ASSERT(id == batch::PolygonBatch::RangeID(-1));
			id = _batch->generate_id(composite, min_range, max_range);
			transformer.pre_get();
			_batch->set_polygon(id, composite, transformer.global());
		}

		void Polygonal::init(math::Polygon2DComposite&& composite, GLushort min_range, GLushort max_range)
		{
			OLY_ASSERT(id == batch::PolygonBatch::RangeID(-1));
			id = _batch->generate_id(composite, min_range, max_range);
			transformer.pre_get();
			_batch->set_polygon(id, std::move(composite), transformer.global());
		}

		void Polygonal::resize(const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range)
		{
			OLY_ASSERT(this->id != batch::PolygonBatch::RangeID(-1));
			_batch->resize_range(id, composite, min_range, max_range);
			transformer.pre_get();
			_batch->set_polygon(id, composite, transformer.global());
		}

		void Polygonal::resize(math::Polygon2DComposite&& composite, GLushort min_range, GLushort max_range)
		{
			OLY_ASSERT(this->id != batch::PolygonBatch::RangeID(-1));
			_batch->resize_range(id, composite, min_range, max_range);
			transformer.pre_get();
			_batch->set_polygon(id, std::move(composite), transformer.global());
		}

		void Polygonal::send_polygon(const math::Polygon2DComposite& composite) const
		{
			auto vertex_range = _batch->get_vertex_range(id);
			auto index_range = _batch->get_index_range(id);
			for (GLushort i = 0; i < vertex_range.length; ++i)
			{
				_batch->set_primitive_points(vertex_range.initial + i, composite[i].polygon.points.data(), (GLushort)composite[i].polygon.points.size());
				_batch->set_primitive_colors(vertex_range.initial + i, composite[i].polygon.colors.data(), (GLushort)composite[i].polygon.colors.size());
				_batch->set_primitive_triangulation(vertex_range.initial + i, index_range.initial + i, composite[i].triangulation);
			}
		}

		void Polygonal::flush() const
		{
			if (transformer.flush())
			{
				transformer.pre_get();
				_batch->set_polygon_transform(id, transformer.global());
			}
		}
		
		void Polygon::init(GLushort min_range, GLushort max_range)
		{
			Polygonal::init(math::split_polygon_composite(polygon, batch()->get_capacity().degree), min_range, max_range);
		}

		void Polygon::resize(GLushort min_range, GLushort max_range)
		{
			Polygonal::resize(math::split_polygon_composite(polygon, batch()->get_capacity().degree), min_range, max_range);
		}
		
		void Polygon::send_polygon() const
		{
			Polygonal::send_polygon(math::split_polygon_composite(polygon, batch()->get_capacity().degree));
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
			math::split_polygon_composite(dup, batch()->get_capacity().degree);
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
