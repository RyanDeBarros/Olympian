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

		oly::rendering::GLBuffer vbo_position;
		oly::rendering::GLBuffer vbo_color;
		oly::rendering::GLBuffer vbo_transform;

		std::vector<glm::vec2> positions;
		std::vector<glm::vec4> colors;
		std::vector<glm::mat3> transforms;

		std::vector<GLushort> indices;

	public:
		typedef GLushort PolygonPos;
		
	private:
		struct PolygonIndex
		{
			PolygonPos index;
			GLushort vertices_offset;
			GLushort num_vertices;
			GLushort indices_offset;
			GLushort num_indices;
		};
		std::vector<PolygonIndex> polygons;

	public:
		struct Capacity
		{
			size_t vertices;
			size_t indices;
		};

	private:
		Capacity capacity;
		
	public:
		PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds);

		void draw() const;

		void set_projection(const glm::vec4& projection_bounds) const;

		void append_polygon(const math::Polygon2D& polygon, const Transform2D& transform);
	};
	
	// TODO
	class EllipseBatch;
}
