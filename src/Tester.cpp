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
	oly::init_context();

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
	
	oly::apollo::SpriteList sprite_list(1000, 5, 2, { -720, 720, -540, 540 });
	enum
	{
		TEX_EINSTEIN = 1,
		TEX_FLAG = 2,
		TEX_TUX = 3
	};
	sprite_list.set_texture(einstein_texture, einstein_texture_dim, TEX_EINSTEIN);
	sprite_list.set_texture(flag_texture, flag_texture_dim, TEX_FLAG);
	sprite_list.set_texture(tux_texture, tux_texture_dim, TEX_TUX);
	sprite_list.set_uvs({ 0,0 }, { 1,0 }, { 1,1 }, { 0,1 }, 0);
	sprite_list.set_uvs({ 0.5f,0 }, { 1,0 }, { 1,1 }, { 0.5f,1 }, 1);
	sprite_list.set_draw_spec(0, 100);
	
	auto& quad0 = sprite_list.get_quad(0);
	quad0.tex_info().tex_slot = TEX_EINSTEIN;
	quad0.transform()[2][0] = 300;
	quad0.transform()[2][1] = 200;
	quad0.send_data();
	
	auto& quad1 = sprite_list.get_quad(1);
	quad1.tex_info().tex_slot = TEX_EINSTEIN;
	quad1.send_tex_info();
	
	auto& quad2 = sprite_list.get_quad(2);
	quad2.tex_info().tex_slot = TEX_TUX;
	quad2.transform()[2][0] = -100;
	quad2.transform()[2][1] = -100;
	quad2.transform()[0][0] = 0.2f;
	quad2.transform()[1][1] = 0.2f;
	quad2.send_data();

	std::vector<oly::apollo::SpriteList::Quad*> flag_tesselation;
	flag_tesselation.reserve(64);
	for (int i = 0; i < 64; ++i)
	{
		flag_tesselation.push_back(&sprite_list.get_quad(3 + i));
		flag_tesselation[i]->tex_info().tex_slot = TEX_FLAG;
		flag_tesselation[i]->transform()[0][0] = 2;
		flag_tesselation[i]->transform()[1][1] = 2;
		flag_tesselation[i]->transform()[2][0] = -160 + (i % 8) * 40;
		flag_tesselation[i]->transform()[2][1] = 160 - (i / 8) * 40;
		flag_tesselation[i]->send_data();
	}
	
	sprite_list.move_quad_order(quad2.index_pos(), 0);
	sprite_list.move_quad_order(quad1.index_pos(), quad1.index_pos() + 40);

	while (!window.should_close())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();

		quad2.transform()[1][0] += 0.001f;
		quad2.send_transform();

		quad1.transform()[0][0] = (float)glm::cos(glfwGetTime());
		quad1.transform()[0][1] = (float)glm::sin(glfwGetTime());
		quad1.transform()[1][0] = (float)-glm::sin(glfwGetTime());
		quad1.transform()[1][1] = (float)glm::cos(glfwGetTime());
		quad1.send_transform();

		static GLushort tex_index = TEX_EINSTEIN;
		if (fmod(glfwGetTime(), 1.0f) < 1.0f / 3.0f)
		{
			if (tex_index != TEX_EINSTEIN)
			{
				tex_index = TEX_EINSTEIN;
				quad0.tex_info().tex_slot = TEX_EINSTEIN;
				quad0.tex_info().tex_coord_slot = 1 - quad0.tex_info().tex_coord_slot;
				quad0.send_tex_info();
			}
		}
		else if (fmod(glfwGetTime(), 1.0f) < 2.0f / 3.0f)
		{
			if (tex_index != TEX_FLAG)
			{
				tex_index = TEX_FLAG;
				quad0.tex_info().tex_slot = TEX_FLAG;
				quad0.tex_info().tex_coord_slot = 1 - quad0.tex_info().tex_coord_slot;
				quad0.send_tex_info();
			}
		}
		else
		{
			if (tex_index != TEX_TUX)
			{
				tex_index = TEX_TUX;
				quad0.tex_info().tex_slot = TEX_TUX;
				quad0.tex_info().tex_coord_slot = 1 - quad0.tex_info().tex_coord_slot;
				quad0.send_tex_info();
			}
		}

		sprite_list.process();
		sprite_list.draw();

		window.swap_buffers();
	}
}
