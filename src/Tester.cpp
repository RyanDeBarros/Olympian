#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include "rendering/core/Batches.h"

struct SpriteVertexData
{
	// vbo 0
	glm::vec2 vertex_positions[4] = {};
	// vbo 1
	glm::mat3 vertex_transforms[4] = { glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f) };
	// vbo 0
	const glm::vec2 vertex_tex_coords[4] = {
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f }
	};
	// vbo 2
	mutable unsigned char vertex_tex_slots[4] = { 0, 0, 0, 0 };

	GLuint texture; // TODO eventually, std::vector<oly::rendering::Texture>

	void load(int w, int h)
	{
		vertex_positions[0].x = -w * 0.5f;
		vertex_positions[0].y = -h * 0.5f;
		vertex_positions[1].x =  w * 0.5f;
		vertex_positions[1].y = -h * 0.5f;
		vertex_positions[2].x =  w * 0.5f;
		vertex_positions[2].y =  h * 0.5f;
		vertex_positions[3].x = -w * 0.5f;
		vertex_positions[3].y =  h * 0.5f;
	}
};

struct SpriteElementData
{
	const unsigned short indices[6] = {
		0, 1, 2,
		2, 3, 0
	};
};

struct SpriteDrawSpecification
{
	GLenum mode = GL_TRIANGLES;
	GLuint indices = 6;
	GLenum type = GL_UNSIGNED_SHORT;
	GLuint offset = 0;
};

template<>
void oly::rendering::draw(const Batch<SpriteVertexData, SpriteElementData, SpriteDrawSpecification>& batch)
{
	const auto& vdata = batch.vertex_data;
	const auto& spec = batch.draw_specification;

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, vdata.texture);
	//glBindTextureUnit(GL_TEXTURE0 + 0, vdata.texture); // TODO only available in 4.5+, so use macros. also explorer bindless textures instead of sampler2d[]

	for (size_t i = 0; i < 4; ++i)
		vdata.vertex_tex_slots[i] = 0;
	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(2));
	glBufferData(GL_ARRAY_BUFFER, sizeof(vdata.vertex_tex_slots), &vdata.vertex_tex_slots, GL_DYNAMIC_DRAW);

	glDrawElements(spec.mode, spec.indices, spec.type, (void*)spec.offset);
}

template<>
void oly::rendering::attrib_layout(const SpriteVertexData&, const std::vector<std::shared_ptr<GLBuffer>>& vbos)
{
	glBindBuffer(GL_ARRAY_BUFFER, *vbos[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) + sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) + sizeof(glm::vec2), (void*)sizeof(glm::vec2));
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, *vbos[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)0);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)sizeof(glm::vec3));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, *vbos[2]);
	glVertexAttribPointer(5, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(unsigned char), (void*)0);
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void init_buffers(const SpriteVertexData& vertex_data, const SpriteElementData& element_data, const std::vector<std::shared_ptr<oly::rendering::GLBuffer>>& vbos, GLuint ebo)
{
	glBindBuffer(GL_ARRAY_BUFFER, *vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, 4 * (sizeof(glm::vec2) + sizeof(glm::vec2)), nullptr, GL_STATIC_DRAW);
	unsigned char* buf = (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	for (size_t i = 0; i < 4; ++i)
	{
		*(glm::vec2*)buf = vertex_data.vertex_positions[i];
		buf += sizeof(glm::vec2);
		*(glm::vec2*)buf = vertex_data.vertex_tex_coords[i];
		buf += sizeof(glm::vec2);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	buf = nullptr;

	glBindBuffer(GL_ARRAY_BUFFER, *vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::mat3), glm::value_ptr(vertex_data.vertex_transforms[0]), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, *vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(unsigned char), &vertex_data.vertex_tex_slots, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned short), element_data.indices, GL_STATIC_DRAW);
}

int main()
{
	if (glfwInit() != GLFW_TRUE)
		return 1;

	stbi_set_flip_vertically_on_load(true);

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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int w, h, chpp;
	unsigned char* image = stbi_load("../../../res/textures/einstein.png", &w, &h, &chpp, 4);
	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLenum internal_format =
		chpp == 1 ? GL_R8
		: chpp == 2 ? GL_RG8
		: chpp == 3 ? GL_RGB8
		: GL_RGBA8;
	GLenum format =
		chpp == 1 ? GL_RED
		: chpp == 2 ? GL_RG
		: chpp == 3 ? GL_RGB
		: GL_RGBA;
	GLint alignment =
		chpp == 1 ? 1
		: chpp == 2 ? 2
		: chpp == 3 ? 1
		: 4;
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, GL_UNSIGNED_BYTE, image);
	stbi_image_free(image);

	oly::rendering::Batch<SpriteVertexData, SpriteElementData, SpriteDrawSpecification> batch;
	batch.shader = oly::rendering::load_shader("../../../src/shaders/sprite_2d.vert", "../../../src/shaders/sprite_2d.frag");
	batch.gen_vao_descriptor(3, true);
	batch.vertex_data.load(w, h);
	batch.vertex_data.texture = texture;
	init_buffers(batch.vertex_data, batch.element_data, batch.vao_descriptor->vbos, *batch.vao_descriptor->ebo);

	glm::mat3 proj = glm::ortho<float>(-720, 720, -540, 540);
	GLuint proj_location = glGetUniformLocation(batch.get_shader(), "uProjection");
	glUseProgram(batch.get_shader());
	glUniformMatrix3fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj));

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glUseProgram(batch.get_shader());
		glBindVertexArray(batch.get_vao());
		oly::rendering::draw(batch);

		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	glDeleteTextures(1, &texture);

	glfwTerminate();
	return 0;
}
