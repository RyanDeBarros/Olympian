#pragma once

#include "core/Core.h"
#include "math/Transforms.h"
#include "math/DataStructures.h"
#include "math/Geometry.h"

namespace oly
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
			size_t vertices = 0;
			size_t indices = 0;
			size_t polygons = 0;
			size_t degree = 0;

			Capacity(size_t polygons, size_t degree)
				: polygons(polygons), degree(degree)
			{
				assert(degree >= 3);
				vertices = polygons * degree;
				indices = polygons * polygon_indices();
			}

			size_t polygon_indices() const { return (degree - 2) * 3; }
		};

	private:
		Capacity capacity;

	public:
		PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds);

		void draw() const;

		void set_projection(const glm::vec4& projection_bounds) const;

		void set_polygon(PolygonPos pos, math::Polygon2D&& polygon, const Transform2D& transform);
		void set_polygon(PolygonPos pos, math::Polygon2D&& polygon, const math::Triangulation& triangulation, const Transform2D& transform);
	};

	// TODO
	class EllipseBatch;
}
