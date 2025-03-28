#pragma once

#include "core/Core.h"
#include "math/Transforms.h"
#include "math/DataStructures.h"
#include "math/Geometry.h"

namespace oly
{
	namespace batch
	{
		class PolygonBatch
		{
			GLuint shader;
			oly::rendering::VertexArray vao;
			oly::rendering::GLBuffer ebo;

			GLuint projection_location;
			GLuint degree_location;

			oly::rendering::GLBuffer vbo_position;
			oly::rendering::GLBuffer vbo_color;

			std::vector<math::Polygon2D> polygons;
			std::vector<glm::mat3> transforms;

			oly::rendering::GLBuffer ssbo_transforms;

			std::vector<GLushort> indices;

		public:
			typedef GLushort PolygonPos;
			GLushort index_offset(PolygonPos pos) const;

		private:
			struct PolygonIndexer
			{
				PolygonPos index;
				GLushort vertices_offset;
				GLushort num_vertices;
				GLushort indices_offset;
				GLushort num_indices;
			};
			std::vector<PolygonIndexer> polygon_indexers;

		public:
			struct Capacity
			{
				GLushort vertices = 0;
				GLushort indices = 0;
				GLushort polygons = 0;
				GLushort degree = 0;

				Capacity(GLushort polygons, GLushort degree)
					: polygons(polygons), degree(degree)
				{
					assert(degree >= 3);
					vertices = polygons * degree;
					indices = polygons * polygon_index_count();
				}

				GLushort polygon_index_count() const;
			};

		private:
			Capacity capacity;

		public:
			GLushort max_degree() const { return capacity.degree; }

			PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void draw() const;

			void set_projection(const glm::vec4& projection_bounds) const;

			void set_polygon(PolygonPos pos, math::Polygon2D&& polygon, const Transform2D& transform);
			void set_polygon(PolygonPos pos, math::Polygon2D&& polygon, const math::Triangulation& triangulation, const Transform2D& transform);
			GLushort set_polygon_composite(PolygonPos pos, const math::TriangulatedPolygon2D& polygon, const Transform2D& transform, GLushort max_range = 0);
			GLushort set_polygon_composite(PolygonPos pos, const math::Polygon2DComposite& composite, const Transform2D& transform, GLushort max_range = 0);
			GLushort set_polygon_composite(PolygonPos pos, math::Polygon2DComposite&& composite, const Transform2D& transform, GLushort max_range = 0);

			math::Polygon2DComposite create_bordered_ngon(PolygonPos pos, glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const;
			math::Polygon2DComposite create_bordered_ngon(PolygonPos pos, glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const;
			math::Polygon2DComposite create_bordered_ngon(PolygonPos pos, const std::vector<glm::vec4>& fill_colors, const std::vector<glm::vec4>& border_colors,
				float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const;
			math::Polygon2DComposite create_bordered_ngon(PolygonPos pos, std::vector<glm::vec4>&& fill_colors, std::vector<glm::vec4>&& border_colors,
				float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const;
		};

		// TODO
		class EllipseBatch;
	}
}
