#pragma once

#include "core/Core.h"
#include "math/Transforms.h"
#include "math/DataStructures.h"
#include "math/Geometry.h"
#include "util/FixedVector.h"

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

			FixedVector<math::Polygon2D> polygons;
			FixedVector<glm::mat3> transforms;

			oly::rendering::GLBuffer ssbo_transforms;

			FixedVector<GLushort> indices;

		public:
			typedef GLushort PrimitivePos;
			typedef GLushort PolygonPos;
			GLushort index_offset(PrimitivePos pos) const;

			struct Capacity
			{
				GLushort vertices = 0;
				GLushort indices = 0;
				GLushort polygons = 0;
				GLushort degree = 0;

				Capacity(GLushort polygons, GLushort degree = 6)
					: polygons(polygons), degree(degree)
				{
					assert(degree >= 3);
					assert(degree * polygons <= USHRT_MAX);
					vertices = polygons * degree;
					indices = polygons * polygon_index_count();
				}

				GLushort polygon_index_count() const;
			};

		private:
			const Capacity capacity;

		public:
			GLushort max_degree() const { return capacity.degree; }

			PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void draw() const;

			void set_projection(const glm::vec4& projection_bounds) const;

		private:
			void set_polygon_primitive(PrimitivePos pos, math::Polygon2D&& polygon, const Transform2D& transform);
			void set_polygon_primitive(PrimitivePos pos, math::Polygon2D&& polygon, const math::Triangulation& triangulation, const Transform2D& transform);
			void disable_polygon_primitive(PrimitivePos pos);
		public:
			void disable_polygon(PolygonPos pos);
			void set_polygon(PolygonPos pos, math::Polygon2D&& polygon, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
			void set_polygon(PolygonPos pos, math::Polygon2D&& polygon, math::Triangulation&& triangulation, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
			void set_polygon(PolygonPos pos, const math::TriangulatedPolygon2D& polygon, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
			void set_polygon(PolygonPos pos, const math::Polygon2DComposite& composite, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
			void set_polygon(PolygonPos pos, math::Polygon2DComposite&& composite, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
			void set_ngon(PolygonPos pos, const math::NGonBase& ngon, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
			void set_bordered_ngon(PolygonPos pos, const math::NGonBase& ngon, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);

			math::Polygon2DComposite create_bordered_ngon(PolygonPos pos, glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const;
			math::Polygon2DComposite create_bordered_ngon(PolygonPos pos, glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const;
			math::Polygon2DComposite create_bordered_ngon(PolygonPos pos, const std::vector<glm::vec4>& fill_colors, const std::vector<glm::vec4>& border_colors,
				float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const;
			math::Polygon2DComposite create_bordered_ngon(PolygonPos pos, std::vector<glm::vec4>&& fill_colors, std::vector<glm::vec4>&& border_colors,
				float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const;
			
			math::Polygon2DComposite create_ngon(PolygonPos pos, const math::NGonBase& ngon) const;
			math::Polygon2DComposite create_bordered_ngon(PolygonPos pos, const math::NGonBase& ngon) const;

		private:
			struct PolygonIndexer
			{
				struct Composite
				{
					PrimitivePos start;
					GLushort range;
				};

				GLushort total_range = 0;
				std::vector<Composite> composites;

				PrimitivePos next_pos() const { return total_range; }
				void register_composite(PrimitivePos start, GLushort range)
				{
					composites.emplace_back(start, range);
					total_range += range;
				}
				PrimitivePos get_pos(PolygonPos pos) const { return composites[pos].start; }
				GLushort get_range(PolygonPos pos) const { return composites[pos].range; }
				bool exists(PolygonPos pos) const { return pos < composites.size(); }
				GLushort size() const { return (GLushort)composites.size(); }
			};
			PolygonIndexer polygon_indexer;

		public:
			PrimitivePos primitive_start(PolygonPos pos) const;
			GLushort primitive_range(PolygonPos pos) const;
		};

		// TODO
		class EllipseBatch;
	}
}
