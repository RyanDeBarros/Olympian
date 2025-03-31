#include "Shapes.h"

#include <glm/gtc/type_ptr.hpp>

#include "Resources.h"
#include "util/General.h"

namespace oly
{
	namespace batch
	{
		GLushort PolygonBatch::Capacity::polygon_index_count() const
		{
			// max(F) = V - 2 + 2H
			// max(H) = [V / 3] - 1
			// --> max(F) = V + 2 * [V / 3] - 4
			// --> return 3 * max(F)
			assert(degree >= 3);
			return 3 * degree + 6 * (degree / 3) - 12;
		}

		PolygonBatch::PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: capacity(capacity), polygons(capacity.polygons), transforms(capacity.polygons), indices(capacity.indices)
		{
			shader = shaders::polygon_batch;
			glUseProgram(shader);
			projection_location = glGetUniformLocation(shader, "uProjection");
			degree_location = glGetUniformLocation(shader, "uDegree");

			glBindVertexArray(vao);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
			glNamedBufferStorage(vbo_position, capacity.vertices * sizeof(glm::vec2), nullptr, GL_DYNAMIC_STORAGE_BIT);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glNamedBufferStorage(vbo_color, capacity.vertices * sizeof(glm::vec4), nullptr, GL_DYNAMIC_STORAGE_BIT);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_transforms);
			glNamedBufferStorage(ssbo_transforms, capacity.polygons * sizeof(glm::mat3), nullptr, GL_DYNAMIC_STORAGE_BIT);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glNamedBufferStorage(ebo, capacity.indices * sizeof(GLushort), indices.data(), GL_DYNAMIC_STORAGE_BIT);

			set_projection(projection_bounds);

			glBindVertexArray(0);
		}

		void PolygonBatch::draw() const
		{
			glUseProgram(shader);
			glUniform1ui(degree_location, capacity.degree);
			glBindVertexArray(vao);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_transforms);
			glDrawElements(GL_TRIANGLES, (GLsizei)capacity.indices, GL_UNSIGNED_SHORT, 0);
		}

		void PolygonBatch::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void PolygonBatch::set_polygon_primitive(PrimitivePos pos, math::Polygon2D&& polygon, const Transform2D& transform)
		{
			set_polygon_primitive(pos, std::move(polygon), math::ear_clipping(polygon.points), transform);
		}

		void PolygonBatch::set_polygon_primitive(PrimitivePos pos, math::Polygon2D&& polygon, const math::Triangulation& triangulation, const Transform2D& transform)
		{
			assert(polygon.valid());
			assert(polygon.points.size() <= capacity.degree);
			assert(triangulation.faces.size() * 3 <= capacity.polygon_index_count());
			assert(pos < capacity.polygons);

			GLushort vertices_offset = pos * capacity.degree;
			GLushort indices_offset = pos * capacity.polygon_index_count();

			polygon.fill_colors();
			polygons[pos] = std::move(polygon);

			transforms[pos] = transform.matrix();
			
			GLushort vertex_index_offset = pos * capacity.degree;
			for (size_t i = 0; i < triangulation.faces.size(); ++i)
			{
				indices[indices_offset + 3 * i + 0] = triangulation.faces[i][0] + vertex_index_offset;
				indices[indices_offset + 3 * i + 1] = triangulation.faces[i][1] + vertex_index_offset;
				indices[indices_offset + 3 * i + 2] = triangulation.faces[i][2] + vertex_index_offset;
			}
			for (size_t i = triangulation.faces.size() * 3; i < capacity.polygon_index_count(); i += 3)
			{
				indices[indices_offset + i + 0] = 0;
				indices[indices_offset + i + 1] = 0;
				indices[indices_offset + i + 2] = 0;
			}

			glNamedBufferSubData(vbo_position, vertices_offset * sizeof(glm::vec2), capacity.degree * sizeof(glm::vec2), polygons[pos].points.data());
			glNamedBufferSubData(vbo_color, vertices_offset * sizeof(glm::vec4), capacity.degree * sizeof(glm::vec4), polygons[pos].colors.data());
			glNamedBufferSubData(ssbo_transforms, pos * sizeof(glm::mat3), sizeof(glm::mat3), transforms.data() + pos);
			glNamedBufferSubData(ebo, indices_offset * sizeof(GLushort), capacity.polygon_index_count() * sizeof(GLushort), indices.data() + indices_offset);
		}

		void PolygonBatch::disable_polygon_primitive(PrimitivePos pos)
		{
			assert(pos < capacity.polygons);

			GLushort vertices_offset = pos * capacity.degree;
			GLushort indices_offset = pos * capacity.polygon_index_count();

			polygons[pos] = {};

			transforms[pos] = {};

			for (size_t i = 0; i < capacity.polygon_index_count(); i += 3)
			{
				indices[indices_offset + i + 0] = 0;
				indices[indices_offset + i + 1] = 0;
				indices[indices_offset + i + 2] = 0;
			}

			glNamedBufferSubData(vbo_position, vertices_offset * sizeof(glm::vec2), capacity.degree * sizeof(glm::vec2), polygons[pos].points.data());
			glNamedBufferSubData(vbo_color, vertices_offset * sizeof(glm::vec4), capacity.degree * sizeof(glm::vec4), polygons[pos].colors.data());
			glNamedBufferSubData(ssbo_transforms, pos * sizeof(glm::mat3), sizeof(glm::mat3), transforms.data() + pos);
			glNamedBufferSubData(ebo, indices_offset * sizeof(GLushort), capacity.polygon_index_count() * sizeof(GLushort), indices.data() + indices_offset);
		}

		void PolygonBatch::disable_polygon(PolygonPos pos)
		{
			if (polygon_indexer.exists(pos))
				for (GLushort diff = 0; diff < polygon_indexer.get_range(pos); ++diff)
					disable_polygon(polygon_indexer.get_pos(pos) + diff);
		}

		void PolygonBatch::set_polygon(PolygonPos pos, math::Polygon2D&& polygon, const Transform2D& transform, GLushort min_range, GLushort max_range)
		{
			math::TriangulatedPolygon2D poly;
			poly.polygon = std::move(polygon);
			poly.triangulation = math::ear_clipping(poly.polygon.points);
			set_polygon(pos, math::split_polygon_composite(std::move(poly), capacity.degree), transform, min_range, max_range);
		}

		void PolygonBatch::set_polygon(PolygonPos pos, math::Polygon2D&& polygon, math::Triangulation&& triangulation, const Transform2D& transform, GLushort min_range, GLushort max_range)
		{
			math::TriangulatedPolygon2D poly;
			poly.polygon = std::move(polygon);
			poly.triangulation = std::move(triangulation);
			set_polygon(pos, math::split_polygon_composite(std::move(poly), capacity.degree), transform, min_range, max_range);
		}

		void PolygonBatch::set_polygon(PolygonPos pos, const math::TriangulatedPolygon2D& polygon, const Transform2D& transform, GLushort min_range, GLushort max_range)
		{
			set_polygon(pos, math::split_polygon_composite(polygon, capacity.degree), transform, min_range, max_range);
		}

		void PolygonBatch::set_polygon(PolygonPos pos, math::TriangulatedPolygon2D&& polygon, const Transform2D& transform, GLushort min_range, GLushort max_range)
		{
			set_polygon(pos, math::split_polygon_composite(std::move(polygon), capacity.degree), transform, min_range, max_range);
		}
		
		void PolygonBatch::set_polygon(PolygonPos pos, const math::Polygon2DComposite& composite, const Transform2D& transform, GLushort min_range, GLushort max_range)
		{
			set_polygon(pos, dupl(composite), transform, min_range, max_range);
		}
		
		void PolygonBatch::set_polygon(PolygonPos pos, math::Polygon2DComposite&& composite, const Transform2D& transform, GLushort min_range, GLushort max_range)
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
				set_polygon_primitive(start + diff++, std::move(poly.polygon), poly.triangulation, transform);
			while (diff < composite_range)
				disable_polygon(start + diff++);
			if (!polygon_indexer.exists(pos))
				polygon_indexer.register_composite(start, composite_range);
		}

		void PolygonBatch::set_ngon(PolygonPos pos, const math::NGonBase& ngon, const Transform2D& transform, GLushort min_range, GLushort max_range)
		{
			set_polygon(pos, create_ngon(pos, ngon), transform, min_range, max_range);
		}

		void PolygonBatch::set_bordered_ngon(PolygonPos pos, const math::NGonBase& ngon, const Transform2D& transform, GLushort min_range, GLushort max_range)
		{
			set_polygon(pos, create_bordered_ngon(pos, ngon), transform, min_range, max_range);
		}

		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(PolygonPos pos, glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const
		{
			return math::create_bordered_ngon(fill_color, border_color, border, border_pivot, points);
		}
		
		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(PolygonPos pos, glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const
		{
			return math::create_bordered_ngon(fill_color, border_color, border, border_pivot, std::move(points));
		}

		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(PolygonPos pos, const std::vector<glm::vec4>& fill_colors, const std::vector<glm::vec4>& border_colors,
			float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const
		{
			return math::create_bordered_ngon(fill_colors, border_colors, border, border_pivot, points);
		}

		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(PolygonPos pos, std::vector<glm::vec4>&& fill_colors, std::vector<glm::vec4>&& border_colors,
			float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const
		{
			return math::create_bordered_ngon(std::move(fill_colors), std::move(border_colors), border, border_pivot, std::move(points));
		}

		math::Polygon2DComposite PolygonBatch::create_ngon(PolygonPos pos, const math::NGonBase& ngon) const
		{
			return ngon.composite(capacity.degree);
		}
		
		math::Polygon2DComposite PolygonBatch::create_bordered_ngon(PolygonPos pos, const math::NGonBase& ngon) const
		{
			return ngon.bordered_composite(capacity.degree);
		}
	}
}
