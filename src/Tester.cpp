#include "Olympian.h"

#include <nanosvg/nanosvg.h>
#include <nanosvg/nanosvgrast.h>

int main()
{
	oly::Context oly_context("../../../res/assets/oly_context.toml");

	oly::Transformer2D flag_tesselation_parent;
	flag_tesselation_parent.modifier = std::make_unique<oly::PivotShearTransformModifier2D>();
	flag_tesselation_parent.local.position.y = -100;
	auto& flag_tesselation_modifier = flag_tesselation_parent.get_modifier<oly::PivotShearTransformModifier2D>();
	flag_tesselation_modifier = { { 0.0f, 0.0f }, { 400, 320 }, { 0, 1 } };
	flag_tesselation_parent.post_set();
	std::vector<oly::rendering::Sprite> flag_tesselation;
	const int flag_rows = 8, flag_cols = 8;
	flag_tesselation.reserve(flag_rows * flag_cols);
	for (int i = 0; i < flag_rows * flag_cols; ++i)
	{
		flag_tesselation.push_back(oly_context.sprite("flag instance"));
		flag_tesselation[i].set_local().position = { -flag_tesselation_modifier.size.x * 0.5f + float(i % flag_cols) * flag_tesselation_modifier.size.x / flag_cols,
			flag_tesselation_modifier.size.y * 0.5f - float(i / flag_rows) * flag_tesselation_modifier.size.y / flag_rows };
		flag_tesselation[i].transformer.attach_parent(&flag_tesselation_parent);
	}
	
	auto concave_shape = oly_context.ref_composite("concave shape").lock();
	for (auto& tp : concave_shape->composite)
	{
		tp.polygon.colors[0].r = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].g = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].b = (float)rand() / RAND_MAX;
	}
	concave_shape->send_polygon();

	auto octagon = oly_context.ref_ngon("octagon").lock();
	auto flag_texture = oly_context.texture_registry().get_texture("flag");

	// LATER begin play on initial actors here

	glEnable(GL_BLEND);
	while (oly_context.frame())
	{
		// logic update

		octagon->base.fill_colors[0].r = fmod(oly::TIME.now<float>(), 1.0f);
		octagon->base.fill_colors[0].b = fmod(oly::TIME.now<float>(), 1.0f);
		octagon->base.border_width = fmod(oly::TIME.now<float>() * 0.05f, 0.1f);
		octagon->base.points[6].x = fmod(oly::TIME.now<float>(), 0.6f) - 0.3f;
		octagon->send_polygon();
	
		concave_shape->set_local().rotation += 0.5f * oly::TIME.delta<float>();

		if (auto sprite1 = oly_context.ref_sprite("sprite1").lock())
			sprite1->set_local().rotation = oly::TIME.now<float>();
		if (auto sprite2 = oly_context.ref_sprite("sprite2").lock())
		{
			sprite2->transformer.get_modifier<oly::ShearTransformModifier2D>().shearing.x += 0.5f * oly::TIME.delta<float>();
			sprite2->transformer.post_set();
		}

		flag_tesselation_modifier.pivot += glm::vec2(0.05f * oly::TIME.delta<float>());
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
				oly_context.sync_texture_handle(flag_texture);
			}
		}
		else
		{
			if (fmod(oly::TIME.now<float>(), 1.0f) < 0.5f)
			{
				lin = true;
				flag_texture->set_and_use_handle(oly::samplers::linear);
				oly_context.sync_texture_handle(flag_texture);
			}
		}

		// draw

		oly::stencil::begin();
		oly::stencil::enable_drawing();
		glClear(GL_STENCIL_BUFFER_BIT); // must be called after enabling stencil drawing
		oly::stencil::draw::replace();
		oly_context.execute_draw_command("polygon crop");
		oly::stencil::disable_drawing();
		oly::stencil::crop::match();
		oly_context.execute_draw_command("sprite match");
		oly::stencil::end();
		oly_context.execute_draw_command("ellipses");
		for (const auto& sprite : flag_tesselation)
			sprite.draw();
		oly_context.render_sprites();
		oly_context.execute_draw_command("sprites and polygons");
	}
}
