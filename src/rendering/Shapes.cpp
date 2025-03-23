#include "Shapes.h"

#include <glm/gtc/type_ptr.hpp>

#include "Resources.h"

oly::PolygonBatch::PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds)
	: capacity(capacity)
{
	// TODO asserts on capacity

	shader = shaders::polygon_batch;
	glUseProgram(shader);
	projection_location = glGetUniformLocation(shader, "uProjection");

	glBindVertexArray(vao);

	positions.resize(capacity.vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glNamedBufferStorage(vbo_position, positions.size() * sizeof(glm::vec2), positions.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	colors.resize(capacity.vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glNamedBufferStorage(vbo_color, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	transforms.resize(capacity.vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_transform);
	glNamedBufferStorage(vbo_transform, transforms.size() * sizeof(glm::mat3), transforms.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(0 * sizeof(glm::vec3)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(1 * sizeof(glm::vec3)));
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	indices.resize(capacity.indices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glNamedBufferStorage(ebo, indices.size() * sizeof(GLushort), indices.data(), GL_DYNAMIC_STORAGE_BIT);

	set_projection(projection_bounds);
}

void oly::PolygonBatch::draw() const
{
	glUseProgram(shader);
	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_SHORT, 0);
}

void oly::PolygonBatch::set_projection(const glm::vec4& projection_bounds) const
{
	glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
	glUseProgram(shader);
	glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
}

void oly::PolygonBatch::append_polygon(const math::Polygon2D& polygon, const Transform2D& transform)
{
	assert(polygon.points.size() >= 3);
	assert(polygon.colors.size() == 1 || polygon.points.size() == polygon.colors.size());

	auto polygon_faces = math::ear_clipping(0, polygon);
	if (polygons.empty())
	{
		PolygonIndex p;
		p.index = 0;
		p.vertices_offset = 0;
		p.num_vertices = (GLushort)polygon.points.size();
		p.indices_offset = 0;
		p.num_indices = (GLushort)polygon_faces.num_indices();
		polygons.push_back(p);
	}
	else
	{
		PolygonIndex p;
		p.index = (PolygonPos)polygons.size();
		p.vertices_offset = polygons.back().vertices_offset + polygons.back().num_vertices;
		p.num_vertices = (GLushort)polygon.points.size();
		p.indices_offset = polygons.back().indices_offset + polygons.back().num_indices;
		p.num_indices = (GLushort)polygon_faces.num_indices();
		polygons.push_back(p);
	}

	positions.insert(positions.begin() + polygons.back().vertices_offset, polygon.points.begin(), polygon.points.end());

	if (polygon.colors.size() > 1)
		colors.insert(colors.begin() + polygons.back().vertices_offset, polygon.colors.begin(), polygon.colors.end());
	else
		for (size_t i = 0; i < polygon.points.size(); ++i)
			colors.insert(colors.begin() + polygons.back().vertices_offset + i, polygon.colors[0]);

	for (size_t i = 0; i < polygon.points.size(); ++i)
		transforms.insert(transforms.begin() + polygons.back().vertices_offset + i, transform.matrix());

	size_t fi = 0;
	for (glm::ivec3 face : polygon_faces.faces)
	{
		indices.insert(indices.begin() + polygons.back().indices_offset + fi++, face[0]);
		indices.insert(indices.begin() + polygons.back().indices_offset + fi++, face[1]);
		indices.insert(indices.begin() + polygons.back().indices_offset + fi++, face[2]);
	}
	
	glNamedBufferSubData(vbo_position, polygons.back().vertices_offset * sizeof(glm::vec2), polygons.back().num_vertices * sizeof(glm::vec2), positions.data() + polygons.back().vertices_offset);
	glNamedBufferSubData(vbo_color, polygons.back().vertices_offset * sizeof(glm::vec4), polygons.back().num_vertices * sizeof(glm::vec4), colors.data() + polygons.back().vertices_offset);
	glNamedBufferSubData(vbo_transform, polygons.back().vertices_offset * sizeof(glm::mat3), polygons.back().num_vertices * sizeof(glm::mat3), transforms.data() + polygons.back().vertices_offset);
	glNamedBufferSubData(ebo, polygons.back().indices_offset * sizeof(GLushort), polygons.back().num_indices * sizeof(GLushort), indices.data() + polygons.back().indices_offset);
}
