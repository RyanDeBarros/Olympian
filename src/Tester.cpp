#include "Olympian.h"

#include "rendering/Sprites.h"
#include "util/Errors.h"
#include "Transforms.h"

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
	glEnable(GL_BLEND);

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

	oly::SpriteList sprite_list({ 1000, 5, 2, 2 }, { -720, 720, -540, 540 });
	enum
	{
		TEX_EINSTEIN = 1,
		TEX_FLAG = 2,
		TEX_TUX = 3
	};
	sprite_list.set_texture(einstein_texture, einstein_texture_dim, TEX_EINSTEIN);
	sprite_list.set_texture(flag_texture, flag_texture_dim, TEX_FLAG);
	sprite_list.set_texture(tux_texture, tux_texture_dim, TEX_TUX);
	sprite_list.set_uvs({ { { 0.5f, 0 }, { 1, 0 }, { 1, 1 }, { 0.5f, 1 } } }, 1);
	sprite_list.set_modulation({ {
		{ 1.0f, 1.0f, 0.2f, 1.0f },
		{ 0.2f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 0.2f, 1.0f, 1.0f },
		{ 0.5f, 0.5f, 0.5f, 1.0f }
		} }, 1);
	sprite_list.set_draw_spec(0, 100);

	oly::Sprite sprite0(&sprite_list, 0);
	sprite0.quad().info().tex_slot = TEX_EINSTEIN;
	sprite0.quad().send_info();
	sprite0.local().position.x = 300;
	sprite0.local().position.y = 300;
	sprite0.post_set();

	oly::Sprite sprite1(&sprite_list, 1);
	sprite1.quad().info().tex_slot = TEX_EINSTEIN;
	sprite1.quad().info().color_slot = 1;
	sprite1.quad().send_info();

	oly::Sprite sprite2(&sprite_list, 2);
	sprite2.quad().info().tex_slot = TEX_TUX;
	sprite2.quad().send_info();
	sprite2.local().position.x = -100;
	sprite2.local().position.y = -100;
	sprite2.local().scale = glm::vec2(0.2f);
	sprite2.post_set();

	oly::PivotTransformer2D flag_tesselation_parent;
	flag_tesselation_parent.pivot = { 0.0f, 0.0f };
	flag_tesselation_parent.size = { 400, 320 };
	int flag_rows = 8;
	int flag_cols = 8;
	flag_tesselation_parent.post_set();
	std::vector<oly::Sprite> flag_tesselation;
	flag_tesselation.reserve(64);
	for (int i = 0; i < flag_rows * flag_cols; ++i)
	{
		flag_tesselation.emplace_back(&sprite_list, 3 + i);
		flag_tesselation[i].quad().info().tex_slot = TEX_FLAG;
		flag_tesselation[i].quad().send_info();
		flag_tesselation[i].local().scale = glm::vec2(2);
		flag_tesselation[i].local().position.x = -flag_tesselation_parent.size.x * 0.5f + float(i % flag_cols) * flag_tesselation_parent.size.x / flag_cols;
		flag_tesselation[i].local().position.y = flag_tesselation_parent.size.y * 0.5f - float(i / flag_rows) * flag_tesselation_parent.size.y / flag_rows;
		flag_tesselation[i].post_set();
		flag_tesselation[i].transformer().attach_parent(&flag_tesselation_parent);
	}
	
	sprite2.quad().set_z_index(0);
	oly::SpriteList::QuadPos z = 0;
	bool inc_z = true;

	while (!window.should_close())
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glfwPollEvents();
	
		sprite1.local().rotation = (float)glfwGetTime();
		sprite1.post_set();
		if (inc_z)
		{
			if (z == 60)
				inc_z = false;
			else
				++z;
		}
		else
		{
			if (z == 3)
				inc_z = true;
			else
				--z;
		}
		sprite1.quad().set_z_index(z);

		sprite2.local().shearing.x += 0.008f;
		sprite2.post_set();

		static GLushort tex_index = TEX_EINSTEIN;
		if (fmod(glfwGetTime(), 1.0f) < 1.0f / 3.0f)
		{
			if (tex_index != TEX_EINSTEIN)
			{
				tex_index = TEX_EINSTEIN;
				sprite0.quad().info().tex_slot = TEX_EINSTEIN;
				sprite0.quad().info().tex_coord_slot = 1 - sprite0.quad().info().tex_coord_slot;
				sprite0.quad().send_info();
			}
		}
		else if (fmod(glfwGetTime(), 1.0f) < 2.0f / 3.0f)
		{
			if (tex_index != TEX_FLAG)
			{
				tex_index = TEX_FLAG;
				sprite0.quad().info().tex_slot = TEX_FLAG;
				sprite0.quad().info().tex_coord_slot = 1 - sprite0.quad().info().tex_coord_slot;
				sprite0.quad().send_info();
			}
		}
		else
		{
			if (tex_index != TEX_TUX)
			{
				tex_index = TEX_TUX;
				sprite0.quad().info().tex_slot = TEX_TUX;
				sprite0.quad().info().tex_coord_slot = 1 - sprite0.quad().info().tex_coord_slot;
				sprite0.quad().send_info();
			}
		}

		flag_tesselation_parent.pivot.x += 0.001f;
		flag_tesselation_parent.pivot.y += 0.001f;
		flag_tesselation_parent.local.rotation -= 0.01f;
		flag_tesselation_parent.post_set();

		sprite_list.process();
		sprite_list.draw();

		window.swap_buffers();
	}
}
