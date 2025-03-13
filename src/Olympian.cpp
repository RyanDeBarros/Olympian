#include "Olympian.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/core/Batches.h"

struct PyramidVertexData
{
	// TODO try interleaving vertex positions with vertex colors
	const float vertex_positions[3 * 4] = {
		 0, 100, -58,
		 0,   0, 115,
		87, -50, -58,
	   -87, -50, -58
	};
	glm::mat4 vertex_transforms[4] = {};
	unsigned char vertex_colors[4 * 4] = {
		255,   0,   0, 255,
		  0, 255,   0, 255,
		255, 255,   0, 255,
		  0,   0, 255, 255
	};

	PyramidVertexData()
	{
		for (glm::mat4& transform : vertex_transforms)
		{
			transform = glm::translate(glm::mat4(1.0f), glm::vec3{ 0.0f, 0.0f, -500.0f }) * glm::scale(glm::mat4(1.0f), glm::vec3(4.0f));
			transform = glm::rotate(transform, glm::radians(60.0f), glm::vec3{ 0.0f, 0.0f, 1.0f });
		}
	}
};

struct PyramidElementData
{
	const unsigned short indices[3 * 4] = {
		0, 1, 2,
		0, 3, 1,
		0, 2, 3,
		1, 3, 2
	};
};

struct PyramidDrawSpecification
{
	GLenum mode = GL_TRIANGLES;
	GLuint indices = 3 * 4;
	GLenum type = GL_UNSIGNED_SHORT;
	GLuint offset = 0;
};

template<>
void oly::rendering::draw(const PyramidDrawSpecification& spec)
{
	glDrawElements(spec.mode, spec.indices, spec.type, (void*)spec.offset);
}

template<>
void oly::rendering::attrib_layout(const PyramidVertexData&, const std::vector<std::shared_ptr<GLBuffer>>& vbos)
{
	glBindBuffer(GL_ARRAY_BUFFER, *vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, *vbos[1]);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(0 * sizeof(glm::vec4)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, *vbos[2]);
	glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
	glEnableVertexAttribArray(5);
}

static void init_buffers(const PyramidVertexData& vertex_data, const PyramidElementData& element_data, const std::vector<std::shared_ptr<oly::rendering::GLBuffer>>& vbos, GLuint ebo)
{
	glBindBuffer(GL_ARRAY_BUFFER, *vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data.vertex_positions), vertex_data.vertex_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, *vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data.vertex_transforms), glm::value_ptr(vertex_data.vertex_transforms[0]), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, *vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data.vertex_colors), vertex_data.vertex_colors, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element_data.indices), element_data.indices, GL_STATIC_DRAW);
}

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

	oly::rendering::Batch<PyramidVertexData, PyramidElementData, PyramidDrawSpecification> batch;
	batch.shader = oly::rendering::load_shader("../../../src/shaders/color_ngon_3d.glsl");
	batch.gen_vao_descriptor(3, true);
	init_buffers(batch.vertex_data, batch.element_data, batch.vao_descriptor->vbos, *batch.vao_descriptor->ebo);

	glm::mat4 proj = glm::ortho<float>(-720.0f, 720.0f, -540.0f, 540.0f, 0.1f, 1000.0f);
	GLuint proj_location = glGetUniformLocation(batch.get_shader(), "uProjection");
	glUseProgram(batch.get_shader());
	glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj));

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		for (glm::mat4& transform : batch.vertex_data.vertex_transforms)
			transform = glm::rotate(transform, 0.01f, glm::vec3{ 0.5f, 0.5f, 0.0f });
		glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(1));
		glm::mat4* gl_map = (glm::mat4*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		for (size_t i = 0; i < 4; ++i)
			gl_map[i] = batch.vertex_data.vertex_transforms[i];
		glUnmapBuffer(GL_ARRAY_BUFFER);
		gl_map = nullptr;

		glUseProgram(batch.get_shader());
		glBindVertexArray(batch.get_vao());
		batch.draw();

		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glfwTerminate();
	return 0;
}
