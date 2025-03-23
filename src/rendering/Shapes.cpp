#include "Shapes.h"

#include <glm/gtc/type_ptr.hpp>

#include "Resources.h"

oly::PolygonBatch::PolygonBatch(const glm::vec4& projection_bounds)
{
	shader = shaders::polygon_batch;
	glUseProgram(shader);
	projection_location = glGetUniformLocation(shader, "uProjection");

	std::vector<glm::vec2> pentagon;
	pentagon.push_back({ 1, -1 });
	pentagon.push_back({ 1, 0 });
	pentagon.push_back({ 0, 1 });
	pentagon.push_back({ -1, 0 });
	pentagon.push_back({ -1, -1 });

	positions.insert(positions.end(), pentagon.begin(), pentagon.end());

	colors.push_back({ 1.0f, 1.0f, 0.0f, 0.5f });
	colors.push_back({ 1.0f, 0.0f, 1.0f, 0.5f });
	colors.push_back({ 0.0f, 1.0f, 1.0f, 0.5f });
	colors.push_back({ 0.0f, 0.0f, 0.0f, 0.5f });
	colors.push_back({ 1.0f, 1.0f, 1.0f, 0.5f });

	Transform2D transform;
	transform.scale = glm::vec2(80);
	for (size_t i = 0; i < pentagon.size(); ++i)
		transforms.push_back(transform.matrix());
	
	auto pentagon_faces = math::ear_clipping(0, pentagon);

	for (glm::ivec3 face : pentagon_faces)
	{
		indices.push_back(face[0]);
		indices.push_back(face[1]);
		indices.push_back(face[2]);
	}

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
