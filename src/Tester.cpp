﻿#include "Olympian.h"

#include "rendering/core/Batch.h"
#include "rendering/core/Textures.h"
#include "rendering/core/Window.h"
#include "util/Errors.h"

#include <iostream>

struct SpriteListCPUData
{
	std::vector<oly::rendering::TextureRes> textures;
	struct TexData
	{
		oly::rendering::BindlessTextureHandle handle;
		glm::vec2 dimensions = {};
	};
	oly::rendering::GLBuffer tex_data_ssbo;
	struct QuadTexInfo
	{
		GLuint tex_slot = 0;
		GLuint tex_coord_slot = 0;
	};
	std::vector<QuadTexInfo> quad_textures;
	oly::rendering::GLBuffer quad_texture_ssbo;
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
		QuadTexInfo* tex_info = nullptr;
		TransformMat3* transform = nullptr;
	};

	SpriteListCPUData(size_t num_textures, size_t num_quads, size_t num_uvs)
	{
		assert(num_uvs <= 500);

		textures.resize(num_textures);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_data_ssbo);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, num_textures * sizeof(TexData), nullptr, GL_DYNAMIC_STORAGE_BIT);

		quad_textures.resize(num_quads);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_texture_ssbo);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_textures.size() * sizeof(QuadTexInfo), quad_textures.data(), GL_DYNAMIC_STORAGE_BIT);

		quad_transforms.resize(num_quads);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_transform_ssbo);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_transforms.size() * sizeof(TransformMat3), quad_transforms.data(), GL_DYNAMIC_STORAGE_BIT);

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

	Quad get_quad(size_t pos)
	{
		Quad quad;
		quad.tex_info = &quad_textures[pos];
		quad.transform = &quad_transforms[pos];
		return quad;
	}

	void send_quad_data(size_t pos)
	{
		glNamedBufferSubData(quad_texture_ssbo, pos * sizeof(TexData), sizeof(TexData), quad_textures.data() + pos);
		glNamedBufferSubData(quad_transform_ssbo, pos * sizeof(TransformMat3), sizeof(TransformMat3), quad_transforms.data() + pos);
	}

	const GLushort indices[12] = {
		4, 5, 6,
		6, 7, 4,
		0, 1, 2,
		2, 3, 0,
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data.quad_texture_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, data.quad_transform_ssbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, data.tex_coords_ubo);
	glDrawElements(data.draw_spec.mode, data.draw_spec.indices, data.draw_spec.type, (void*)(data.draw_spec.offset*sizeof(GLushort)));
}

template<>
void oly::rendering::attrib_layout(const SpriteListBatch& batch)
{
}

static void run();

int main()
{
	oly::init();
	run();
	return oly::terminate();
}

void run()
{
	oly::rendering::WindowHint hint;
	hint.context.clear_color = { 0.2f, 0.5f, 0.8f, 1.0f };
	oly::rendering::Window window(1440, 1080, "Olympian Engine", hint);

	oly::rendering::ImageDimensions einstein_texture_dim;
	auto einstein_texture = oly::rendering::load_static_texture_2d("../../../res/textures/einstein.png", einstein_texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	oly::rendering::ImageDimensions flag_texture_dim;
	auto flag_texture = oly::rendering::load_static_texture_2d("../../../res/textures/flag.png", flag_texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	oly::rendering::ImageDimensions tux_texture_dim;
	auto tux_texture = oly::rendering::load_static_texture_2d("../../../res/textures/tux.png", tux_texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	SpriteListBatch batch(3, 10, 2);
	batch.shader = oly::rendering::load_shader("../../../src/shaders/sprite_2d.vert", "../../../src/shaders/sprite_2d.frag");
	batch.gen_vao_descriptor(0, true);
	glNamedBufferData(batch.get_ebo(), batch.cpu_data.draw_spec.indices * sizeof(GLushort), batch.cpu_data.indices, GL_STATIC_DRAW);
	batch.cpu_data.set_texture(einstein_texture, einstein_texture_dim, 0);
	batch.cpu_data.set_texture(flag_texture, flag_texture_dim, 1);
	batch.cpu_data.set_texture(tux_texture, tux_texture_dim, 2);
	batch.cpu_data.set_uvs({ 0,0 }, { 1,0 }, { 1,1 }, { 0,1 }, 0);
	batch.cpu_data.set_uvs({ 0.5f,0 }, { 1,0 }, { 1,1 }, { 0.5f,1 }, 1);
	auto quad0 = batch.cpu_data.get_quad(0);
	auto quad1 = batch.cpu_data.get_quad(1);
	quad1.transform->v2[0] = 300;
	quad1.transform->v2[1] = 200;
	batch.cpu_data.send_quad_data(1);

	glm::mat3 proj = glm::ortho<float>(-720, 720, -540, 540);
	GLuint proj_location = glGetUniformLocation(batch.get_shader(), "uProjection");
	glUseProgram(batch.get_shader());
	glUniformMatrix3fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj));

	while (!window.should_close())
	{
		glfwPollEvents();

		quad0.transform->v0[0] = (float)glm::cos(glfwGetTime());
		quad0.transform->v0[1] = (float)glm::sin(glfwGetTime());
		quad0.transform->v1[0] = (float)-glm::sin(glfwGetTime());
		quad0.transform->v1[1] = (float)glm::cos(glfwGetTime());
		batch.cpu_data.send_quad_data(0);

		static GLushort tex_index = 0;
		if (fmod(glfwGetTime(), 1.0f) < 1.0f / 3.0f)
		{
			if (tex_index != 0)
			{
				tex_index = 0;
				quad1.tex_info->tex_slot = 0;
				quad1.tex_info->tex_coord_slot = 1 - quad1.tex_info->tex_coord_slot;
				batch.cpu_data.send_quad_data(1);
			}
		}
		else if (fmod(glfwGetTime(), 1.0f) < 2.0f / 3.0f)
		{
			if (tex_index != 1)
			{
				tex_index = 1;
				quad1.tex_info->tex_slot = 1;
				quad1.tex_info->tex_coord_slot = 1 - quad1.tex_info->tex_coord_slot;
				batch.cpu_data.send_quad_data(1);
			}
		}
		else
		{
			if (tex_index != 2)
			{
				tex_index = 2;
				quad1.tex_info->tex_slot = 2;
				quad1.tex_info->tex_coord_slot = 1 - quad1.tex_info->tex_coord_slot;
				batch.cpu_data.send_quad_data(2);
			}
		}

		glUseProgram(batch.get_shader());
		glBindVertexArray(batch.get_vao());
		oly::rendering::draw(batch);

		window.swap_buffers();
		glClear(GL_COLOR_BUFFER_BIT);
	}
}
