#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include "rendering/core/Batches.h"
#include "rendering/core/Textures.h"

#include <iostream>

struct SpriteCPUData
{
	// vbo 0
	GLushort vertex_tex_slots[4] = { 0, 0, 0, 0 };
	// vbo 1
	GLushort vertex_tex_coord_slots[4] = { 0, 0, 0, 0 };
	// vbo 2
	glm::mat3 vertex_transforms[4] = { glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f) };

	std::vector<oly::rendering::TextureRes> textures;
	struct TexData
	{
		oly::rendering::BindlessTextureHandle handle;
		glm::vec2 dimensions = {};
	};
	oly::rendering::GLBuffer tex_data_ssbo;
	struct TexUVRect
	{
		glm::vec2 uvs[4];
	};
	oly::rendering::GLBuffer tex_coords_ubo;

	void init(size_t num_textures, size_t num_uvs)
	{
		assert(num_uvs <= 500);
		textures.resize(num_textures);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_data_ssbo);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, num_textures * sizeof(TexData), nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBuffer(GL_UNIFORM_BUFFER, tex_coords_ubo);
		glBufferStorage(GL_UNIFORM_BUFFER, num_uvs * sizeof(TexUVRect), nullptr, GL_DYNAMIC_STORAGE_BIT);
	}

	void set_texture(const oly::rendering::TextureRes& texture, oly::rendering::ImageDimensions dim, size_t pos)
	{
		textures[pos] = texture;
		TexData texture_data;
		texture_data.dimensions = { dim.w, dim.h };
		texture_data.handle.use(*texture);
		glNamedBufferSubData(tex_data_ssbo, pos * sizeof(TexData), sizeof(TexData), &texture_data);

	}

	void set_uvs(glm::vec2 bl, glm::vec2 br, glm::vec2 tr, glm::vec2 tl, size_t pos)
	{
		TexUVRect texture_coords;
		texture_coords.uvs[0] = bl;
		texture_coords.uvs[1] = br;
		texture_coords.uvs[2] = tr;
		texture_coords.uvs[3] = tl;
		glNamedBufferSubData(tex_coords_ubo, pos * sizeof(TexUVRect), sizeof(TexUVRect), &texture_coords);
	}

	const unsigned short indices[6] = {
		0, 1, 2,
		2, 3, 0
	};

	struct
	{
		GLenum mode = GL_TRIANGLES;
		GLuint indices = 6;
		GLenum type = GL_UNSIGNED_SHORT;
		GLuint offset = 0;
	} draw_spec;
};

typedef oly::rendering::Batch<SpriteCPUData> SpriteBatch;

template<>
void oly::rendering::draw(const SpriteBatch& batch)
{
	const auto& data = batch.cpu_data;

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data.tex_data_ssbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, data.tex_coords_ubo);
	glDrawElements(data.draw_spec.mode, data.draw_spec.indices, data.draw_spec.type, (void*)data.draw_spec.offset);
}

template<>
void oly::rendering::attrib_layout(const SpriteBatch& batch)
{
	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(0));
	glVertexAttribPointer(0, 1, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(GLubyte), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(1));
	glVertexAttribPointer(1, 1, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(GLubyte), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(2));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)0);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)sizeof(glm::vec3));
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void init_buffers(const SpriteBatch& batch)
{
	const auto& data = batch.cpu_data;

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(0));
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLubyte), &data.vertex_tex_slots, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(1));
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLubyte), &data.vertex_tex_coord_slots, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, batch.get_vbo(2));
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::mat3), glm::value_ptr(data.vertex_transforms[0]), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.get_ebo());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned short), data.indices, GL_STATIC_DRAW);
}

int main()
{
	if (glfwInit() != GLFW_TRUE)
		return 1;

	stbi_set_flip_vertically_on_load(true);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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

	oly::rendering::ImageDimensions texture_dim;
	auto texture = oly::rendering::load_static_texture_2d("../../../res/textures/einstein.png", texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	SpriteBatch batch;
	batch.shader = oly::rendering::load_shader("../../../src/shaders/sprite_2d.vert", "../../../src/shaders/sprite_2d.frag");
	batch.gen_vao_descriptor(3, true);
	batch.cpu_data.init(10, 3);
	batch.cpu_data.set_texture(texture, texture_dim, 0);
	batch.cpu_data.set_uvs({0,0}, {1,0}, {1,1}, {0,1}, 0);
	init_buffers(batch);

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

	glfwTerminate();
	return 0;
}
