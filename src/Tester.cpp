#include "Olympian.h"

#include "rendering/Sprites.h"
#include "rendering/Shapes.h"
#include "rendering/Resources.h"
#include "util/Errors.h"
#include "util/General.h"

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
	oly::load_context();
	glEnable(GL_BLEND);

	oly::rendering::ImageDimensions einstein_texture_dim;
	auto einstein_texture = oly::rendering::load_static_bindless_texture_2d("../../../res/textures/einstein.png", einstein_texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	einstein_texture->set_handle();
	oly::rendering::ImageDimensions flag_texture_dim;
	auto flag_texture = oly::rendering::load_static_bindless_texture_2d("../../../res/textures/flag.png", flag_texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	flag_texture->set_handle();
	oly::rendering::ImageDimensions tux_texture_dim;
	auto tux_texture = oly::rendering::load_static_bindless_texture_2d("../../../res/textures/tux.png", tux_texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	tux_texture->set_handle();

	oly::batch::SpriteBatch sprite_batch({ 1000, 5, 2, 2 }, { -720, 720, -540, 540 });
	enum
	{
		TEX_EINSTEIN = 1,
		TEX_FLAG = 2,
		TEX_TUX = 3
	};
	sprite_batch.set_texture(TEX_EINSTEIN, einstein_texture, einstein_texture_dim);
	sprite_batch.set_texture(TEX_FLAG, flag_texture, flag_texture_dim);
	sprite_batch.set_texture(TEX_TUX, tux_texture, tux_texture_dim);
	sprite_batch.set_uvs(1, { { { 0.5f, 0 }, { 1, 0 }, { 1, 1 }, { 0.5f, 1 } } });
	sprite_batch.set_modulation(1, { {
		{ 1.0f, 1.0f, 0.2f, 0.5f },
		{ 0.2f, 1.0f, 1.0f, 0.5f },
		{ 1.0f, 0.2f, 1.0f, 0.5f },
		{ 0.5f, 0.5f, 0.5f, 0.5f }
		} });
	sprite_batch.set_draw_spec(0, 100);

	oly::renderable::Sprite sprite0(&sprite_batch, 0);
	sprite0.quad().info().tex_slot = TEX_EINSTEIN;
	sprite0.quad().send_info();
	sprite0.local().position.x = 300;
	sprite0.local().position.y = 300;
	sprite0.post_set();

	oly::renderable::Sprite sprite1(&sprite_batch, 1);
	sprite1.quad().info().tex_slot = TEX_EINSTEIN;
	sprite1.quad().info().color_slot = 1;
	sprite1.quad().send_info();

	oly::renderable::Sprite sprite2(&sprite_batch, 2);
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
	std::vector<oly::renderable::Sprite> flag_tesselation;
	flag_tesselation.reserve(64);
	for (int i = 0; i < flag_rows * flag_cols; ++i)
	{
		flag_tesselation.emplace_back(&sprite_batch, 3 + i);
		flag_tesselation[i].quad().info().tex_slot = TEX_FLAG;
		flag_tesselation[i].quad().send_info();
		flag_tesselation[i].local().scale = glm::vec2(2);
		flag_tesselation[i].local().position.x = -flag_tesselation_parent.size.x * 0.5f + float(i % flag_cols) * flag_tesselation_parent.size.x / flag_cols;
		flag_tesselation[i].local().position.y = flag_tesselation_parent.size.y * 0.5f - float(i / flag_rows) * flag_tesselation_parent.size.y / flag_rows;
		flag_tesselation[i].post_set();
		flag_tesselation[i].transformer().attach_parent(&flag_tesselation_parent);
	}

	sprite2.quad().set_z_index(0);

	oly::batch::PolygonBatch polygon_batch(oly::batch::PolygonBatch::Capacity(100, 5), { -720, 720, -540, 540 });

	oly::math::Polygon2D pentagon;
	pentagon.points = {
		{ 1, -1 },
		{ 1, 0 },
		{ 0, 1 },
		{ -1, 0 },
		{ -1, -1 }
	};
	pentagon.colors = {
		{ 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};

	oly::Transform2D pentagon_transform;
	pentagon_transform.scale = glm::vec2(160);
	polygon_batch.set_polygon(0, oly::dupl(pentagon), pentagon_transform);
	pentagon_transform.position.x = -250;
	pentagon_transform.rotation = -1;
	pentagon_transform.scale.x *= 2;
	for (glm::vec4& color : pentagon.colors)
		color.a = 0.5f;
	polygon_batch.set_polygon(1, oly::dupl(pentagon), pentagon_transform);

	auto triangle_pos = 2;
	auto triangle = oly::math::create_bordered_triangle({ 0.9f, 0.9f, 0.7f, 1.0f }, { 0.3f, 0.15f, 0.0f, 1.0f }, 0.1f, oly::math::BorderPivot::MIDDLE, { 3, -1 }, { 0, 2 }, { -3, -1 }, polygon_batch.max_degree(), polygon_batch.index_offset(triangle_pos));
	oly::Transform2D triangle_transform;
	triangle_transform.position.x = 100;
	triangle_transform.position.y = -100;
	triangle_transform.scale = glm::vec2(150);
	auto triangle_range = polygon_batch.set_polygon_composite(triangle_pos, triangle, triangle_transform);

	oly::math::NGonBase octagon;
	octagon.fill_colors = { { 0.0f, 1.0f, 0.0f, 0.7f } };
	octagon.border_colors = {
		{ 0.25f,  0.0f,  0.5f, 1.0f },
		{  0.0f, 0.25f, 0.25f, 1.0f },
		{ 0.25f,  0.5f,  0.0f, 1.0f },
		{  0.5f, 0.75f, 0.25f, 1.0f },
		{ 0.75f,  1.0f,  0.5f, 1.0f },
		{  1.0f, 0.75f, 0.75f, 1.0f },
		{ 0.75f,  0.5f,  1.0f, 1.0f },
		{  0.5f, 0.25f, 0.75f, 1.0f }
	};
	octagon.points = {
		{ 1, 0 },
		{ 0.707f, 0.707f },
		{ 0, 1 },
		{ -0.707f, 0.707f },
		{ -1, 0 },
		{ -0.707f, -0.707f },
		{ 0, -1 },
		{ 0.707f, -0.707f }
	};
	octagon.border_width = 0.05f;
	oly::batch::PolygonBatch::PolygonPos octagon_pos = triangle_pos + triangle_range;
	oly::Transform2D octagon_transform;
	octagon_transform.position.x = 300;
	octagon_transform.position.y = 200;
	octagon_transform.scale = glm::vec2(200);
	auto octagon_range = polygon_batch.set_bordered_ngon_composite(octagon_pos, octagon, octagon_transform);

	// TODO create index-tracking system for polygon batch and sprite batch that keeps track of what indices+ranges (for composite polygons) are available.

	while (!window.should_close())
	{
		if (auto err = glGetError())
			__debugbreak();

		glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glfwPollEvents();

		octagon.fill_colors[0].r = fmod((float)glfwGetTime(), 1.0f);
		octagon.fill_colors[0].b = fmod((float)glfwGetTime(), 1.0f);
		octagon.border_width = fmod((float)glfwGetTime() * 0.05f, 0.1f);
		octagon.points[6].x = fmod((float)glfwGetTime(), 0.6f) - 0.3f;
		polygon_batch.set_bordered_ngon_composite(octagon_pos, octagon, octagon_transform, octagon_range, octagon_range);
	
		sprite1.local().rotation = (float)glfwGetTime();
		sprite1.post_set();

		sprite2.local().shearing.x += 0.008f;
		sprite2.post_set();

		flag_tesselation_parent.pivot.x += 0.001f;
		flag_tesselation_parent.pivot.y += 0.001f;
		flag_tesselation_parent.local.rotation -= 0.01f;
		flag_tesselation_parent.post_set();

		sprite_batch.process();
		
		oly::stencil::begin();
		oly::stencil::enable_drawing();
		oly::stencil::draw::replace();
		polygon_batch.draw();
		oly::stencil::disable_drawing();
		oly::stencil::crop::match();
		sprite_batch.set_draw_spec(0, 3);
		sprite_batch.draw();
		oly::stencil::end();
		sprite_batch.set_draw_spec(3, 97);
		sprite_batch.draw();

		window.swap_buffers();
	}
}
