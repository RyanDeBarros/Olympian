#include "Shapes.h"

#include <glm/gtc/type_ptr.hpp>

#include "Resources.h"

oly::PolygonBatch::PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds)
	: capacity(capacity)
{
	assert(capacity.degree >= 3);
	// TODO asserts on capacity

	shader = shaders::polygon_batch;
	glUseProgram(shader);
	projection_location = glGetUniformLocation(shader, "uProjection");
	degree_location = glGetUniformLocation(shader, "uDegree");

	glBindVertexArray(vao);

	polygon_indexers.resize(capacity.polygons);
	polygons.resize(capacity.polygons);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glNamedBufferStorage(vbo_position, capacity.vertices * sizeof(glm::vec2), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glNamedBufferStorage(vbo_color, capacity.vertices * sizeof(glm::vec4), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	transforms.resize(capacity.polygons);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_transforms);
	glNamedBufferStorage(ssbo_transforms, capacity.polygons * sizeof(glm::mat3), nullptr, GL_DYNAMIC_STORAGE_BIT);

	indices.resize(capacity.indices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glNamedBufferStorage(ebo, capacity.indices * sizeof(GLushort), indices.data(), GL_DYNAMIC_STORAGE_BIT);

	set_projection(projection_bounds);

	glBindVertexArray(0);
}

void oly::PolygonBatch::draw() const
{
	glUseProgram(shader);
	glUniform1ui(degree_location, capacity.degree);
	glBindVertexArray(vao);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_transforms);
	glDrawElements(GL_TRIANGLES, (GLsizei)capacity.indices, GL_UNSIGNED_SHORT, 0);
}

void oly::PolygonBatch::set_projection(const glm::vec4& projection_bounds) const
{
	glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
	glUseProgram(shader);
	glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
}

void oly::PolygonBatch::set_polygon(PolygonPos pos, math::Polygon2D&& polygon, const Transform2D& transform)
{
	set_polygon(pos, std::move(polygon), math::ear_clipping(glm::uint(pos * capacity.degree), polygon), transform);
}

void oly::PolygonBatch::set_polygon(PolygonPos pos, math::Polygon2D&& polygon, const math::Triangulation& triangulation, const Transform2D& transform)
{
	assert(polygon.valid());
	assert(polygon.points.size() <= capacity.degree);
	assert(triangulation.num_indices() <= capacity.polygon_indices());
	assert(triangulation.index_offset == pos * capacity.degree); // TODO create method that returns pos * capacity.degree so it can be used externally
	// TODO add asserts for capacity

	PolygonPos polygon_insertion_index = pos;
	GLushort vertex_insertion_index = GLushort(pos * capacity.degree);
	GLushort index_insertion_index = GLushort(pos * capacity.polygon_indices());

	PolygonIndexer p;
	p.index = polygon_insertion_index;
	p.num_vertices = (GLushort)polygon.points.size();
	p.vertices_offset = vertex_insertion_index;
	p.num_indices = (GLushort)triangulation.num_indices();
	p.indices_offset = index_insertion_index;
	polygon_indexers[polygon_insertion_index] = p;

	polygon.fill_colors();
	polygons[polygon_insertion_index] = std::move(polygon);

	transforms[polygon_insertion_index] = transform.matrix();

	for (size_t i = 0; i < triangulation.faces.size(); ++i)
	{
		indices[index_insertion_index + 3 * i + 0] = triangulation.faces[i][0];
		indices[index_insertion_index + 3 * i + 1] = triangulation.faces[i][1];
		indices[index_insertion_index + 3 * i + 2] = triangulation.faces[i][2];
	}

	glNamedBufferSubData(vbo_position, vertex_insertion_index * sizeof(glm::vec2), capacity.degree * sizeof(glm::vec2), polygons[polygon_insertion_index].points.data());
	glNamedBufferSubData(vbo_color, vertex_insertion_index * sizeof(glm::vec4), capacity.degree * sizeof(glm::vec4), polygons[polygon_insertion_index].colors.data());
	glNamedBufferSubData(ssbo_transforms, polygon_insertion_index * sizeof(glm::mat3), sizeof(glm::mat3), transforms.data() + polygon_insertion_index);
	glNamedBufferSubData(ebo, index_insertion_index * sizeof(GLushort), capacity.polygon_indices() * sizeof(GLushort), indices.data() + index_insertion_index);
}
