#include "Olympian.h"

#include "rendering/apollo/Sprites.h"
#include "util/Errors.h"

#include <iostream>

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
	
	oly::apollo::SpriteListBatch batch(10, 5, 2);
	batch.shader = oly::rendering::load_shader("../../../src/shaders/sprite_2d.vert", "../../../src/shaders/sprite_2d.frag");
	enum
	{
		TEX_EINSTEIN = 1,
		TEX_FLAG = 2,
		TEX_TUX = 3
	};
	batch.set_texture(einstein_texture, einstein_texture_dim, TEX_EINSTEIN);
	batch.set_texture(flag_texture, flag_texture_dim, TEX_FLAG);
	batch.set_texture(tux_texture, tux_texture_dim, TEX_TUX);
	batch.set_uvs({ 0,0 }, { 1,0 }, { 1,1 }, { 0,1 }, 0);
	batch.set_uvs({ 0.5f,0 }, { 1,0 }, { 1,1 }, { 0.5f,1 }, 1);
	auto quad0 = batch.get_quad(0);
	quad0.tex_info->tex_slot = TEX_EINSTEIN;
	(*quad0.transform)[2][0] = 300;
	(*quad0.transform)[2][1] = 200;
	batch.send_quad_data(0);
	auto quad1 = batch.get_quad(1);
	quad1.tex_info->tex_slot = TEX_EINSTEIN;
	batch.send_quad_data(1);

	glm::mat3 proj = glm::ortho<float>(-720, 720, -540, 540);
	GLuint proj_location = glGetUniformLocation(*batch.shader, "uProjection");
	glUseProgram(*batch.shader);
	glUniformMatrix3fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj));

	while (!window.should_close())
	{
		glfwPollEvents();

		(*quad1.transform)[0][0] = (float)glm::cos(glfwGetTime());
		(*quad1.transform)[0][1] = (float)glm::sin(glfwGetTime());
		(*quad1.transform)[1][0] = (float)-glm::sin(glfwGetTime());
		(*quad1.transform)[1][1] = (float)glm::cos(glfwGetTime());
		batch.send_quad_data(1);

		static GLushort tex_index = TEX_EINSTEIN;
		if (fmod(glfwGetTime(), 1.0f) < 1.0f / 3.0f)
		{
			if (tex_index != TEX_EINSTEIN)
			{
				tex_index = TEX_EINSTEIN;
				quad0.tex_info->tex_slot = TEX_EINSTEIN;
				quad0.tex_info->tex_coord_slot = 1 - quad0.tex_info->tex_coord_slot;
				batch.send_quad_data(0);
			}
		}
		else if (fmod(glfwGetTime(), 1.0f) < 2.0f / 3.0f)
		{
			if (tex_index != TEX_FLAG)
			{
				tex_index = TEX_FLAG;
				quad0.tex_info->tex_slot = TEX_FLAG;
				quad0.tex_info->tex_coord_slot = 1 - quad0.tex_info->tex_coord_slot;
				batch.send_quad_data(0);
			}
		}
		else
		{
			if (tex_index != TEX_TUX)
			{
				tex_index = TEX_TUX;
				quad0.tex_info->tex_slot = TEX_TUX;
				quad0.tex_info->tex_coord_slot = 1 - quad0.tex_info->tex_coord_slot;
				batch.send_quad_data(0);
			}
		}

		batch.draw();

		window.swap_buffers();
		glClear(GL_COLOR_BUFFER_BIT);
	}
}
