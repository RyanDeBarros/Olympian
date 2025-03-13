#include "Olympian.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/core/Batches.h"

int main()
{
	if (glfwInit() != GLFW_TRUE)
		return 1;

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	GLFWwindow* window = glfwCreateWindow(1440, 1080, "Olympian Engine", nullptr, nullptr);
	if (!window)
		return 2;

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		return 3;
	glClearColor(0.2f, 0.5f, 0.8f, 1.0f);
	glfwSwapInterval(1);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	//auto shader = oly::rendering::load_shader("../../../src/shaders/color_ngon_3d.vert", "../../../src/shaders/color_ngon_3d.frag");
	auto shader = oly::rendering::load_shader("../../../src/shaders/color_ngon_3d.glsl");

	oly::rendering::VAODescriptor vao_descriptor{};
	vao_descriptor.vbos = oly::rendering::gen_bulk_buffers(3);
	GLuint vbo_position = vao_descriptor.get_vbo(0);
	GLuint vbo_transform = vao_descriptor.get_vbo(1);
	GLuint vbo_color = vao_descriptor.get_vbo(2);
	vao_descriptor.gen_ebo();

	// TODO try interleaving vertex positions with vertex colors
	const float vertex_positions[3 * 4] = {
		  0, 100, -58,
		  0,   0, 115,
		 87, -50, -58,
		-87, -50, -58
	};

	glm::mat4 vertex_transforms[4] = {};
	for (glm::mat4& transform : vertex_transforms)
	{
		transform = glm::translate(glm::mat4(1.0f), glm::vec3{0.0f, 0.0f, -500.0f}) * glm::scale(glm::mat4(1.0f), glm::vec3(4.0f));
		transform = glm::rotate(transform, glm::radians(60.0f), glm::vec3{ 0.0f, 0.0f, 1.0f });
	}
	
	// TODO try normalized unsigned char for colors
	float vertex_colors[4 * 4] = {
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f
	};

	const unsigned short indices[3 * 4] = {
		0, 1, 2,
		0, 3, 1,
		0, 2, 3,
		1, 3, 2
	};

	glBindVertexArray(vao_descriptor.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_transform);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_transforms), glm::value_ptr(vertex_transforms[0]), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(0 * sizeof(glm::vec4)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_colors), vertex_colors, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(5);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao_descriptor.get_ebo());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glm::mat4 proj = glm::ortho<float>(-720.0f, 720.0f, -540.0f, 540.0f, 0.1f, 1000.0f);
	GLuint proj_location = glGetUniformLocation(shader, "uProjection");
	glUseProgram(shader);
	glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj));

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		for (glm::mat4& transform : vertex_transforms)
			transform = glm::rotate(transform, 0.01f, glm::vec3{ 0.5f, 0.5f, 0.0f });
		glBindBuffer(GL_ARRAY_BUFFER, vbo_transform);
		glm::mat4* gl_map = (glm::mat4*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		for (size_t i = 0; i < 4; ++i)
			gl_map[i] = vertex_transforms[i];
		glUnmapBuffer(GL_ARRAY_BUFFER);
		gl_map = nullptr;

		glUseProgram(shader);
		glBindVertexArray(vao_descriptor.vao);
		glDrawElements(GL_TRIANGLES, 3 * 4, GL_UNSIGNED_SHORT, (void*)0);

		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glfwTerminate();
	return 0;
}
