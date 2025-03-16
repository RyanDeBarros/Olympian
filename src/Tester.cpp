#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include "rendering/core/Batches.h"
#include "rendering/core/Textures.h"

#include <iostream>

struct SpriteListCPUData
{
	// TODO interleave tex slots and coords into one VBO
	// vbo 0
	//GLushort vertex_tex_slots[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	//GLushort vertex_tex_slots[2] = { 0, 0 };
	// vbo 1
	//GLushort vertex_tex_coord_slots[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	//GLushort vertex_tex_coord_slots[2] = { 0, 0 };
	// vbo 2
	//glm::mat3 vertex_transforms[8] = { glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f) };
	//glm::mat3 vertex_transforms[2] = { glm::mat3(1.0f), glm::mat3(1.0f) };

	std::vector<oly::rendering::TextureRes> textures;
	struct TexData
	{
		oly::rendering::BindlessTextureHandle handle;
		glm::vec2 dimensions = {};
	};
	oly::rendering::GLBuffer tex_data_ssbo;
	struct QuadTexInfo
	{
		GLuint tex_slot;
		GLuint tex_coord_slot;
	};
	std::vector<QuadTexInfo> quad_textures;
	oly::rendering::GLBuffer quad_tex_ssbo;
	struct TransformMat3
	{
		alignas(16) glm::vec3 v0 = { 1.0f, 0.0f, 0.0f }, v1 = { 0.0f, 1.0f, 0.0f }, v2 = { 0.0f, 0.0f, 1.0f };
		glm::mat3 get() const
		{
			return { v0, v1, v2 };
		}
		void set(const glm::mat3& mat)
		{
			v0 = mat[0];
			v1 = mat[1];
			v2 = mat[2];
		}
	};
	std::vector<TransformMat3> quad_transforms;
	oly::rendering::GLBuffer quad_transform_ssbo;
	struct TexUVRect
	{
		glm::vec2 uvs[4] = {};
	};
	oly::rendering::GLBuffer tex_coords_ubo;

	struct Quad
	{
		QuadTexInfo* tex_info;
		glm::mat3* transform;
	};

	void init(size_t num_textures, size_t num_uvs, size_t num_quads)
	{
		assert(num_uvs <= 500);
		textures.resize(num_textures);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_data_ssbo);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, num_textures * sizeof(TexData), nullptr, GL_DYNAMIC_STORAGE_BIT);

		quad_textures.resize(num_quads);
		quad_textures[0].tex_slot = 0;
		quad_textures[0].tex_coord_slot = 0;
		quad_textures[1].tex_slot = 0;
		quad_textures[1].tex_coord_slot = 0;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_tex_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, quad_textures.size() * sizeof(QuadTexInfo), quad_textures.data(), GL_STATIC_DRAW);

		quad_transforms.resize(num_quads);
		quad_transforms[1].v2[0] = 300;
		quad_transforms[1].v2[1] = 200;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_transform_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, quad_transforms.size() * sizeof(TransformMat3), quad_transforms.data(), GL_DYNAMIC_DRAW);

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

	const GLushort indices[12] = {
		0, 1, 2,
		2, 3, 0,
		4, 5, 6,
		6, 7, 4
	};

	struct
	{
		GLenum mode = GL_TRIANGLES;
		GLuint indices = 12;
		GLenum type = GL_UNSIGNED_SHORT;
		GLuint offset = 0;
	} draw_spec;
};

typedef oly::rendering::Batch<SpriteListCPUData> SpriteListBatch;

template<>
void oly::rendering::draw(const SpriteListBatch& batch)
{
	const auto& data = batch.cpu_data;

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data.tex_data_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data.quad_tex_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, data.quad_transform_ssbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, data.tex_coords_ubo);
	glDrawElements(data.draw_spec.mode, data.draw_spec.indices, data.draw_spec.type, (void*)(data.draw_spec.offset*sizeof(GLushort)));
}

template<>
void oly::rendering::attrib_layout(const SpriteListBatch& batch)
{
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
	
	SpriteListBatch* batch = new SpriteListBatch();
	batch->shader = oly::rendering::load_shader("../../../src/shaders/sprite_2d.vert", "../../../src/shaders/sprite_2d.frag");
	//batch->gen_vao_descriptor(3, true);
	batch->gen_vao_descriptor(0, true);
	batch->cpu_data.init(10, 3, 2);
	batch->cpu_data.set_texture(texture, texture_dim, 0);
	batch->cpu_data.set_uvs({0,0}, {1,0}, {1,1}, {0,1}, 0);
	//init_buffers(*batch);

	glBindVertexArray(batch->get_vao());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->get_ebo());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, batch->cpu_data.draw_spec.indices * sizeof(GLushort), batch->cpu_data.indices, GL_STATIC_DRAW);

	glm::mat3 proj = glm::ortho<float>(-720, 720, -540, 540);
	GLuint proj_location = glGetUniformLocation(batch->get_shader(), "uProjection");
	glUseProgram(batch->get_shader());
	glUniformMatrix3fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj));
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glUseProgram(batch->get_shader());
		glBindVertexArray(batch->get_vao());
		oly::rendering::draw(*batch);

		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	delete batch;

	glfwTerminate();
	return 0;
}
