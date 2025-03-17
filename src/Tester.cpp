#include "Olympian.h"

#include "rendering/apollo/Sprites.h"
#include "util/Errors.h"
#include "util/Geometry.h"

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
	oly::math::Mat3 mat0;
	mat0.position().x = 300;
	mat0.position().y = 300;
	quad0.transform() = mat0.matrix();
	quad0.send_data();
	
	auto& quad1 = sprite_list.get_quad(1);
	oly::math::Mat3 mat1;
	quad1.tex_info().tex_slot = TEX_EINSTEIN;
	quad1.send_tex_info();
	
	auto& quad2 = sprite_list.get_quad(2);
	quad2.tex_info().tex_slot = TEX_TUX;
	oly::math::Mat3 mat2(oly::math::TransformType::AFFINE);
	mat2.position().x = -100;
	mat2.position().y = -100;
	mat2.scale() = glm::vec2(0.2f);
	quad2.transform() = mat2.matrix();
	quad2.send_data();

	std::vector<oly::apollo::SpriteList::Quad*> flag_tesselation;
	flag_tesselation.reserve(64);
	for (int i = 0; i < 64; ++i)
	{
		flag_tesselation.push_back(&sprite_list.get_quad(3 + i));
		flag_tesselation[i]->tex_info().tex_slot = TEX_FLAG;
		oly::math::Mat3 mat(oly::math::TransformType::FLAT);
		mat.scale() = glm::vec2(2);
		mat.position().x = -160.0f + float(i % 8) * 40.0f;
		mat.position().y = 160.0f - float(i / 8) * 40.0f;
		flag_tesselation[i]->transform() = mat.matrix();
		flag_tesselation[i]->send_data();
	}
	
	sprite_list.move_quad_order(quad2.index_pos(), 0);
	sprite_list.move_quad_order(quad1.index_pos(), quad1.index_pos() + 40);

	while (!window.should_close())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();

		mat2.shearing().x += 0.005f;
		quad2.transform() = mat2.matrix();
		quad2.send_transform();

		mat1.rotation() = (float)glfwGetTime();
		quad1.transform() = mat1.matrix();
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
