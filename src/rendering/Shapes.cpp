#include "Shapes.h"

#include <glm/gtc/type_ptr.hpp>

#include "Resources.h"

oly::PolygonBatch::PolygonBatch(const glm::vec4& projection_bounds)
{
	shader = shaders::polygon_batch();
	glUseProgram(*shader);
	projection_location = glGetUniformLocation(*shader, "uProjection");

	positions.push_back({ 0, 0 });
	positions.push_back({ 1, -1 });
	positions.push_back({ 1, 0 });
	positions.push_back({ 0, 1 });
	positions.push_back({ -1, 0 });
	positions.push_back({ -1, -1 });

	colors.push_back({ 0.5f, 0.5f, 0.5f, 0.5f });
	colors.push_back({ 1.0f, 1.0f, 0.0f, 0.5f });
	colors.push_back({ 1.0f, 0.0f, 1.0f, 0.5f });
	colors.push_back({ 0.0f, 1.0f, 1.0f, 0.5f });
	colors.push_back({ 0.0f, 0.0f, 0.0f, 0.5f });
	colors.push_back({ 1.0f, 1.0f, 1.0f, 0.5f });

	Transform2D transform;
	transform.scale = glm::vec2(80);
	transforms.push_back(transform.matrix());
	transforms.push_back(transform.matrix());
	transforms.push_back(transform.matrix());
	transforms.push_back(transform.matrix());
	transforms.push_back(transform.matrix());
	transforms.push_back(transform.matrix());

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(0);
	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(0);
	indices.push_back(5);
	indices.push_back(1);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glNamedBufferStorage(vbo_position, positions.size() * sizeof(glm::vec2), positions.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glNamedBufferStorage(vbo_color, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_transform);
	glNamedBufferStorage(vbo_transform, transforms.size() * sizeof(glm::mat3), transforms.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(0 * sizeof(glm::vec3)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(1 * sizeof(glm::vec3)));
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glNamedBufferStorage(ebo, indices.size() * sizeof(GLushort), indices.data(), GL_DYNAMIC_STORAGE_BIT);

	set_projection(projection_bounds);
}

void oly::PolygonBatch::draw() const
{
	glUseProgram(*shader);
	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
}

void oly::PolygonBatch::set_projection(const glm::vec4& projection_bounds) const
{
	glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
	glUseProgram(*shader);
	glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
}
