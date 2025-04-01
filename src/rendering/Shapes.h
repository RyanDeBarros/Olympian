#pragma once

#include "SpecializedBuffers.h"
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
			rendering::VertexArray vao;
			rendering::FixedLayoutEBO<GLushort, 1> ebo;

			GLuint projection_location;
			GLuint degree_location;

			enum PolygonAttribute
			{
				POSITION,
				COLOR
			};
			rendering::LazyVertexBufferBlock2x1<glm::vec2, glm::vec4, GLushort> polygon_vbo;

			rendering::IndexedSSBO<glm::mat3, GLushort> transform_ssbo;

		public:
			typedef GLushort PrimitivePos;
			typedef GLushort PolygonPos;

			struct Capacity
			{
				GLushort vertices = 0;
				GLushort indices = 0;
				GLushort polygons = 0;
				GLushort degree = 0;
				GLushort polygon_index_count = 0;

				Capacity(GLushort polygons, GLushort degree = 6)
					: polygons(polygons), degree(degree)
				{
					assert(degree >= 3);
					assert(degree * polygons <= USHRT_MAX);

					// max(F) = V - 2 + 2H
					// max(H) = [V / 3] - 1
					// --> max(F) = V + 2 * [V / 3] - 4
					// --> return 3 * max(F)
					polygon_index_count = 3 * degree + 6 * (degree / 3) - 12;
					vertices = polygons * degree;
					indices = polygons * polygon_index_count;
				}
			};

		private:
			const Capacity capacity;

		public:
			GLushort max_degree() const { return capacity.degree; }

			PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void draw() const;

			void get_primitive_draw_spec(PrimitivePos& first, PrimitivePos& count) const;
			void set_primitive_draw_spec(PrimitivePos first, PrimitivePos count);

			void set_projection(const glm::vec4& projection_bounds) const;

		private:
			void set_polygon_primitive(PrimitivePos pos, const math::Polygon2D& polygon, const Transform2D& transform);
			void set_polygon_primitive(PrimitivePos pos, const math::Polygon2D& polygon, const math::Triangulation& triangulation, const Transform2D& transform);
			void disable_polygon_primitive(PrimitivePos pos);
		public:
			void disable_polygon(PolygonPos pos);
			void set_polygon(PolygonPos pos, math::Polygon2D&& polygon, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
			void set_polygon(PolygonPos pos, math::Polygon2D&& polygon, math::Triangulation&& triangulation, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
			void set_polygon(PolygonPos pos, const math::TriangulatedPolygon2D& polygon, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
			void set_polygon(PolygonPos pos, math::TriangulatedPolygon2D&& polygon, const Transform2D& transform, GLushort min_range = 0, GLushort max_range = 0);
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
			const PolygonIndexer& get_indexer() const { return polygon_indexer; }

			void flush() const;
		};

		// TODO
		class EllipseBatch;
	}
}
