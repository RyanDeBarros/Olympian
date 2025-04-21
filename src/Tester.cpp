#include "Olympian.h"

#include "rendering/mutable/Sprites.h"
#include "rendering/immutable/Polygons.h"
#include "rendering/immutable/Ellipses.h"
#include "rendering/Loader.h"
#include "rendering/Resources.h"

static std::string RES_DIR = "../../../res/";
static std::string TEXTURES_DIR = RES_DIR + "textures/";
static std::string ASSET_DIR = RES_DIR + "assets/";

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
	auto einstein_texture = oly::rendering::load_bindless_texture_2d(TEXTURES_DIR + "einstein.png", einstein_texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	einstein_texture->set_and_use_handle();
	oly::rendering::ImageDimensions flag_texture_dim;
	auto flag_texture = oly::rendering::load_bindless_texture_2d(TEXTURES_DIR + "flag.png", flag_texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	flag_texture->set_and_use_handle();
	oly::rendering::ImageDimensions tux_texture_dim;
	auto tux_texture = oly::rendering::load_bindless_texture_2d(TEXTURES_DIR + "tux.png", tux_texture_dim);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	tux_texture->set_and_use_handle();
	oly::rendering::GIFDimensions serotonin_texture_dim;
	auto serotonin_texture = oly::rendering::load_bindless_texture_2d_array(TEXTURES_DIR + "serotonin.gif", serotonin_texture_dim);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	serotonin_texture->set_and_use_handle();

	oly::mut::SpriteBatch sprite_batch({ 0, 0 }, window.projection_bounds());

	oly::mut::Sprite sprite0(&sprite_batch);
	sprite0.set_texture(einstein_texture, einstein_texture_dim.dimensions());
	sprite0.local().position.x = 300;
	sprite0.local().position.y = 300;
	sprite0.post_set();

	oly::mut::Sprite sprite1(&sprite_batch);
	sprite1.set_texture(einstein_texture, einstein_texture_dim.dimensions());
	sprite1.set_modulation({ {
		{ 1.0f, 1.0f, 0.2f, 0.7f },
		{ 0.2f, 1.0f, 1.0f, 0.7f },
		{ 1.0f, 0.2f, 1.0f, 0.7f },
		{ 0.5f, 0.5f, 0.5f, 0.7f }
	} });

	oly::mut::Sprite sprite2(&sprite_batch);
	sprite2.transformer.modifier = std::make_unique<oly::ShearTransformModifier2D>();
	sprite2.set_texture(tux_texture, tux_texture_dim.dimensions());
	sprite2.local().position.x = -100;
	sprite2.local().position.y = -100;
	sprite2.local().scale = glm::vec2(0.2f);
	sprite2.post_set();

	oly::mut::Sprite sprite3(sprite2);
	sprite3.local().position.x = 100;
	sprite3.local().scale = glm::vec2(2.0f);
	sprite3.post_set();
	sprite3.set_texture(serotonin_texture, { serotonin_texture_dim.w, serotonin_texture_dim.h });
	sprite3.set_frame_format(oly::rendering::setup_gif_frame_format(serotonin_texture_dim));

	oly::Transformer2D flag_tesselation_parent;
	flag_tesselation_parent.modifier = std::make_unique<oly::PivotShearTransformModifier2D>();
	flag_tesselation_parent.local.position.y = -100;
	auto& flag_tesselation_modifier = flag_tesselation_parent.get_modifier<oly::PivotShearTransformModifier2D>();
	flag_tesselation_modifier.pivot = { 0.0f, 0.0f };
	flag_tesselation_modifier.size = { 400, 320 };
	flag_tesselation_modifier.shearing = { 0, 1 };
	flag_tesselation_parent.post_set();
	std::vector<oly::mut::Sprite> flag_tesselation;
	int flag_rows = 8;
	int flag_cols = 8;
	flag_tesselation.reserve(flag_rows * flag_cols);
	for (int i = 0; i < flag_rows * flag_cols; ++i)
	{
		flag_tesselation.emplace_back(&sprite_batch);
		flag_tesselation[i].set_texture(flag_texture, flag_texture_dim.dimensions());
		flag_tesselation[i].local().scale = glm::vec2(2);
		flag_tesselation[i].local().position.x = -flag_tesselation_modifier.size.x * 0.5f + float(i % flag_cols) * flag_tesselation_modifier.size.x / flag_cols;
		flag_tesselation[i].local().position.y = flag_tesselation_modifier.size.y * 0.5f - float(i / flag_rows) * flag_tesselation_modifier.size.y / flag_rows;
		flag_tesselation[i].post_set();
		flag_tesselation[i].transformer.attach_parent(&flag_tesselation_parent);
	}

	oly::immut::PolygonBatch polygon_batch({ 100, 4 }, window.projection_bounds());

	oly::immut::Polygon pentagon1(&polygon_batch);
	pentagon1.polygon.points = {
		{ 1, -1 },
		{ 1, 0 },
		{ 0, 1 },
		{ -1, 0 },
		{ -1, -1 }
	};
	pentagon1.polygon.colors = {
		{ 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	};
	pentagon1.transformer.local.position.y = 200;
	pentagon1.transformer.local.scale = glm::vec2(160);
	pentagon1.post_set();
	pentagon1.init();

	oly::immut::Polygon pentagon2(&polygon_batch);
	pentagon2.polygon = pentagon1.polygon;
	pentagon2.transformer.local.position.x = -250;
	pentagon2.transformer.local.rotation = -1;
	pentagon2.transformer.local.scale.x = 320;
	pentagon2.transformer.local.scale.y = 160;
	pentagon2.post_set();
	for (glm::vec4& color : pentagon2.polygon.colors)
		color.a = 0.5f;
	pentagon2.init();

	oly::immut::Composite bordered_triangle(&polygon_batch);
	bordered_triangle.composite = oly::math::create_bordered_triangle({ 0.9f, 0.9f, 0.7f, 1.0f }, { 0.3f, 0.15f, 0.0f, 1.0f }, 0.1f, oly::math::BorderPivot::MIDDLE, { 3, -1 }, { 0, 2 }, { -3, -1 });
	bordered_triangle.transformer.local.position.x = 100;
	bordered_triangle.transformer.local.position.y = -100;
	bordered_triangle.transformer.local.scale = glm::vec2(150);
	bordered_triangle.post_set();
	bordered_triangle.init();
	bordered_triangle.composite = oly::math::create_bordered_quad({ 0.9f, 0.9f, 0.7f, 1.0f }, { 0.3f, 0.15f, 0.0f, 1.0f }, 0.1f, oly::math::BorderPivot::MIDDLE, { 3, -1 }, { 0, 2 }, { -3, -1 }, { 0, 0 });
	bordered_triangle.resize();

	oly::immut::NGon octagon(&polygon_batch);
	octagon.local() = { { 300, 200 }, 0, { 200, 200 } };
	octagon.base.fill_colors = { { 0.0f, 1.0f, 0.0f, 0.7f } };
	octagon.base.border_colors = {
		{ 0.25f,  0.0f,  0.5f, 1.0f },
		{  0.0f, 0.25f, 0.25f, 1.0f },
		{ 0.25f,  0.5f,  0.0f, 1.0f },
		{  0.5f, 0.75f, 0.25f, 1.0f },
		{ 0.75f,  1.0f,  0.5f, 1.0f },
		{  1.0f, 0.75f, 0.75f, 1.0f },
		{ 0.75f,  0.5f,  1.0f, 1.0f },
		{  0.5f, 0.25f, 0.75f, 1.0f }
	};
	octagon.base.points = {
		{ 1, 0 },
		{ 0.707f, 0.707f },
		{ 0, 1 },
		{ -0.707f, 0.707f },
		{ -1, 0 },
		{ -0.707f, -0.707f },
		{ 0, -1 },
		{ 0.707f, -0.707f }
	};
	octagon.base.border_width = 0.05f;
	octagon.bordered = true;
	octagon.init();

	oly::immut::Composite concave_shape(&polygon_batch);
	concave_shape.local() = { { -200, 200 }, 0, glm::vec2(60) };
	concave_shape.composite = oly::math::composite_convex_decomposition({
		{ -4,  0 },
		{ -2, -2 },
		{  0, -2 },
		{  2, -1 },
		{  4,  1 },
		{  2,  3 },
		{  1,  3 },
		{ -1,  0 },
		{ -3,  1 },
		{ -3,  2 }
	});
	for (auto& tp : concave_shape.composite)
	{
		tp.polygon.colors[0].r = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].g = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].b = (float)rand() / RAND_MAX;
	}
	concave_shape.init();

	polygon_batch.move_poly_order_after(pentagon2.get_id(), bordered_triangle.get_id());
	polygon_batch.move_poly_order_before(concave_shape.get_id(), bordered_triangle.get_id());

	polygon_batch.draw_specs.resize(3);
	{
		oly::immut::PolygonBatch::RangeID post_octagon_id = -1;
		OLY_ASSERT(polygon_batch.get_next_draw_id(octagon.get_id(), post_octagon_id));
		auto post_octagon_range = polygon_batch.get_index_range(post_octagon_id);
		polygon_batch.draw_specs[1] = { 0, post_octagon_range.initial };
		polygon_batch.draw_specs[2] = { post_octagon_range.initial, polygon_batch.get_capacity().indices };
	}

	oly::immut::EllipseBatch ellipse_batch({ 100 }, window.projection_bounds());

	oly::immut::Ellipse ellipse1(&ellipse_batch);
	ellipse1.ellipse.dimension() = { 2, 1, 0.3f, 1.0f, 1.0f };
	ellipse1.ellipse.color().fill_inner = { 1.0f, 0.9f, 0.8f, 0.5f };
	ellipse1.ellipse.color().fill_outer = { 1.0f, 0.6f, 0.2f, 1.0f };
	ellipse1.ellipse.color().border_inner = { 0.2f, 0.6f, 1.0f, 1.0f };
	ellipse1.ellipse.color().border_outer = { 0.8f, 0.9f, 1.0f, 1.0f };
	ellipse1.local() = { { -300, 0 }, 0, { 150, 150 } };
	ellipse1.post_set();
	ellipse1.ellipse.send_data();
	
	oly::immut::Ellipse ellipse2(&ellipse_batch);
	ellipse2.ellipse.dimension() = { 1, 3, 0.4f, 0.5f, 2.0f };
	ellipse2.ellipse.color().fill_inner = { 1.0f, 0.9f, 0.8f, 0.5f };
	ellipse2.ellipse.color().fill_outer = { 1.0f, 0.6f, 0.2f, 1.0f };
	ellipse2.ellipse.color().border_inner = { 0.0f, 0.0f, 0.0f, 1.0f };
	ellipse2.ellipse.color().border_outer = { 0.8f, 0.9f, 1.0f, 0.0f };
	ellipse2.local() = { { 0, 0 }, 0, { 150, 150 } };
	ellipse2.post_set();
	ellipse2.ellipse.send_data();
	ellipse2.ellipse.z_value = -1.0f;
	ellipse2.ellipse.send_z_value();

	while (!window.should_close())
	{
		// pre-frame
		oly::check_errors();
		oly::LOG.flush();
		glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glfwPollEvents();
		oly::TIME.sync();

		// logic update
		octagon.base.fill_colors[0].r = fmod(oly::TIME.now<float>(), 1.0f);
		octagon.base.fill_colors[0].b = fmod(oly::TIME.now<float>(), 1.0f);
		octagon.base.border_width = fmod(oly::TIME.now<float>() * 0.05f, 0.1f);
		octagon.base.points[6].x = fmod(oly::TIME.now<float>(), 0.6f) - 0.3f;
		octagon.send_polygon();
	
		concave_shape.transformer.local.rotation += 0.5f * oly::TIME.delta<float>();
		concave_shape.post_set();

		sprite1.local().rotation = oly::TIME.now<float>();
		sprite1.post_set();

		sprite2.transformer.get_modifier<oly::ShearTransformModifier2D>().shearing.x += 0.5f * oly::TIME.delta<float>();
		sprite2.post_set();

		flag_tesselation_modifier.pivot.x += 0.05f * oly::TIME.delta<float>();
		flag_tesselation_modifier.pivot.y += 0.05f * oly::TIME.delta<float>();
		flag_tesselation_parent.local.rotation -= 0.5f * oly::TIME.delta<float>();
		flag_tesselation_parent.post_set();
		flag_tesselation_parent.flush();

		static bool lin = true;
		if (lin)
		{
			if (fmod(oly::TIME.now<float>(), 1.0f) >= 0.5f)
			{
				lin = false;
				flag_texture->set_and_use_handle(oly::samplers::nearest);
				sprite_batch.update_texture_handle(flag_texture);
			}
		}
		else
		{
			if (fmod(oly::TIME.now<float>(), 1.0f) < 0.5f)
			{
				lin = true;
				flag_texture->set_and_use_handle(oly::samplers::linear);
				sprite_batch.update_texture_handle(flag_texture);
			}
		}
		
		// flush buffers
		polygon_batch.flush();
		ellipse_batch.flush();

		// update particle systems
		
		// draw
		ellipse_batch.draw();
		oly::stencil::begin();
		oly::stencil::enable_drawing();
		oly::stencil::draw::replace();
		polygon_batch.draw(1);
		oly::stencil::disable_drawing();
		oly::stencil::crop::match();
		sprite0.draw();
		sprite1.draw();
		sprite2.draw();
		sprite_batch.render();
		oly::stencil::end();
		for (const auto& sprite : flag_tesselation)
			sprite.draw();
		sprite3.draw();
		sprite_batch.render();
		polygon_batch.draw(2);

		// post-frame
		window.swap_buffers();
	}
}
