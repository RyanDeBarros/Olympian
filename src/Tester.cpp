#include "Olympian.h"

#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/SAT.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "graphics/extensions/Arrow.h" // TODO loading for Arrow and Line extensions. Rename registries folder to assets
#include "physics/collision/debugging/CollisionView.h"

#include "archetypes/PolygonCrop.h"
#include "archetypes/SpriteMatch.h"
#include "archetypes/EllipsePair.h"
#include "archetypes/Jumble.h"
#include "archetypes/BKG.h"

#include "PlayerController.h"

struct KeyHandler : public oly::EventHandler<oly::input::KeyEventData>
{
	virtual bool consume(const oly::input::KeyEventData& data) override
	{
		if (data.action == GLFW_PRESS)
		{
			if (data.key == GLFW_KEY_A)
				oly::LOG << "A" << oly::LOG.endl;
			else if (data.key == GLFW_KEY_B)
				oly::LOG << "B" << oly::LOG.endl;
			else
				oly::LOG << "?" << oly::LOG.endl;
		}
		return false;
	}
};

int main()
{
	oly::context::Context oly_context("../../../res/context.toml");

	PlayerController pc;
	oly::get_platform().bind_signal("jump", &PlayerController::jump, pc);
	oly::get_platform().bind_signal("click", &PlayerController::click, pc);
	oly::get_platform().bind_signal("drag", &PlayerController::drag, pc);
	oly::get_platform().bind_signal("zoom camera", &PlayerController::zoom_camera, pc);

	oly::LOG << oly::get_platform().gamepad().connected() << oly::LOG.endl;
	oly::LOG << oly::get_platform().gamepad().has_mapping() << oly::LOG.endl;

	KeyHandler key_handler;
	key_handler.attach(&oly::get_platform().window().handlers.key);

	oly::Transformer2D flag_tesselation_parent;
	flag_tesselation_parent.modifier = std::make_unique<oly::PivotShearTransformModifier2D>();
	flag_tesselation_parent.local.position.y = -100;
	auto& flag_tesselation_modifier = flag_tesselation_parent.get_modifier<oly::PivotShearTransformModifier2D>();
	flag_tesselation_modifier = { { 0.0f, 0.0f }, { 400, 320 }, { 0, 1 } };
	flag_tesselation_parent.post_set();
	std::vector<oly::Sprite> flag_tesselation;
	const int flag_rows = 8, flag_cols = 8;
	flag_tesselation.reserve(flag_rows * flag_cols);
	oly::Sprite flag_instance = oly::reg::load_sprite(oly::context::load_toml("assets/sprites/flag instance.toml")["sprite"]);
	for (int i = 0; i < flag_rows * flag_cols; ++i)
	{
		flag_tesselation.push_back(flag_instance);
		flag_tesselation[i].set_local().position = { -flag_tesselation_modifier.size.x * 0.5f + float(i % flag_cols) * flag_tesselation_modifier.size.x / flag_cols,
			flag_tesselation_modifier.size.y * 0.5f - float(i / flag_rows) * flag_tesselation_modifier.size.y / flag_rows };
		flag_tesselation[i].transformer.attach_parent(&flag_tesselation_parent);
	}

	oly::gen::BKG bkg;
	oly::gen::PolygonCrop polygon_crop;
	oly::gen::SpriteMatch sprite_match;
	oly::gen::EllipsePair ellipse_pair;
	oly::gen::Jumble jumble;

	pc.test_text = &jumble.test_text;

	for (auto& tp : jumble.concave_shape.composite)
	{
		tp.polygon.colors[0].r = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].g = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].b = (float)rand() / RAND_MAX;
	}
	jumble.concave_shape.send_polygon();

	auto flag_texture = oly::context::load_texture("textures/flag.png");

	oly::col2d::AABB aabb{ .x1 = -300.0f, .x2 = 100.0f, .y1 = -400.0f, .y2 = 500.0f };
	auto aabb_visual = oly::context::polygon();
	aabb_visual.polygon.points = {
		{ aabb.x1, aabb.y1 },
		{ aabb.x2, aabb.y1 },
		{ aabb.x2, aabb.y2 },
		{ aabb.x1, aabb.y2 }
	};
	aabb_visual.polygon.colors = { oly::colors::MAGENTA };
	aabb_visual.init();

	oly::col2d::Circle circ({}, 50.0f);
	auto circ_visual = oly::context::ellipse();
	{
		auto& dimension = circ_visual.ellipse.set_dimension();
		dimension.ry = dimension.rx = circ.radius;
		dimension.fill_exp = 0.0f;
	}
	circ_visual.ellipse.set_color().fill_outer = oly::colors::YELLOW;

	oly::col2d::AABB rect{};
	auto rect_visual = oly::context::polygon();
	rect_visual.polygon.points = { { -circ.radius, -circ.radius }, { circ.radius, -circ.radius }, { circ.radius, circ.radius }, { -circ.radius, circ.radius } };
	rect_visual.polygon.colors = { oly::colors::YELLOW };
	rect_visual.init();

	// LATER anti-aliasing settings
	
	oly::rendering::ArrowExtension aabb_impulse_visual;
	aabb_impulse_visual.set_color(oly::colors::WHITE);
	aabb_impulse_visual.adjust_standard_head_for_width(6.0f);

	oly::rendering::ArrowExtension circ_impulse_visual;
	circ_impulse_visual.set_color(oly::colors::GREEN);
	circ_impulse_visual.adjust_standard_head_for_width(6.0f);

	bool draw_impulse_visual = true;

	oly::CallbackStateTimer flag_state_timer({ 0.5f, 0.5f }, [flag_texture](size_t state) {
		if (state == 0)
			flag_texture->set_and_use_handle(oly::graphics::samplers::nearest);
		else if (state == 1)
			flag_texture->set_and_use_handle(oly::graphics::samplers::linear);
		oly::context::sync_texture_handle(flag_texture);
		});
	
	// LATER begin play on initial actors here

	glEnable(GL_BLEND);

	std::function<void()> render_frame = [&]() {
		bkg.draw(true);
		oly::stencil::begin();
		oly::stencil::enable_drawing();
		glClear(GL_STENCIL_BUFFER_BIT); // must be called after enabling stencil drawing
		oly::stencil::draw::replace();
		polygon_crop.draw(true);
		oly::stencil::disable_drawing();
		oly::stencil::crop::match();
		sprite_match.draw(true);
		oly::stencil::end();
		ellipse_pair.draw(true);
		for (const auto& sprite : flag_tesselation)
			sprite.draw();
		oly::context::render_sprites();
		jumble.draw(true);

		aabb_visual.draw();
		//rect_visual.draw();
		oly::context::render_polygons();
		circ_visual.draw();
		oly::context::render_ellipses();
		if (draw_impulse_visual)
		{
			aabb_impulse_visual.draw();
			circ_impulse_visual.draw();
			oly::context::render_polygons();
		}
		};

	oly::context::set_render_function(oly::make_functor(render_frame));

	while (oly::context::frame())
	{
		// logic update

		circ.center = oly::context::get_cursor_screen_pos();
		auto contact = oly::col2d::gjk::contacts(circ, aabb); // TODO this breaks when circle comes into AABB from left or top.
		//rect = { .x1 = circ.center.x - circ.radius, .x2 = circ.center.x + circ.radius, .y1 = circ.center.y - circ.radius, .y2 = circ.center.y + circ.radius };
		//auto contact = oly::acm2d::contacts(rect, aabb);
		if (contact.overlap)
		{
			circ_visual.ellipse.set_color().fill_outer = oly::colors::RED;
			//rect_visual.polygon.colors[0] = oly::colors::RED;
			draw_impulse_visual = true;
		}
		else
		{
			circ_visual.ellipse.set_color().fill_outer = oly::colors::YELLOW;
			//rect_visual.polygon.colors[0] = oly::colors::YELLOW;
			draw_impulse_visual = false;
		}
		circ_visual.set_local().position = circ.center;
		//rect_visual.send_polygon();
		//rect_visual.set_local().position = circ.center;

		circ_impulse_visual.set_start() = contact.active_feature.position;
		circ_impulse_visual.set_end() = contact.active_feature.position + contact.active_feature.impulse;
		
		aabb_impulse_visual.set_start() = contact.static_feature.position;
		aabb_impulse_visual.set_end() = contact.static_feature.position + contact.static_feature.impulse;

		jumble.nonant_panel.set_width(jumble.nonant_panel.width() - 10.0f * oly::TIME.delta<float>());

		jumble.octagon.base.fill_colors[0].r = fmod(oly::TIME.now<float>(), 1.0f);
		jumble.octagon.base.fill_colors[0].b = fmod(oly::TIME.now<float>(), 1.0f);
		jumble.octagon.base.border_width = fmod(oly::TIME.now<float>() * 0.05f, 0.1f);
		jumble.octagon.base.points[6].x = fmod(oly::TIME.now<float>(), 0.6f) - 0.3f;
		jumble.octagon.send_polygon();

		jumble.concave_shape.set_local().rotation += 0.5f * oly::TIME.delta<float>();

		jumble.sprite1.set_local().rotation = oly::TIME.now<float>();
		sprite_match.sprite2.transformer.get_modifier<oly::ShearTransformModifier2D>().shearing.x += 0.5f * oly::TIME.delta<float>();
		sprite_match.sprite2.transformer.post_set();
		
		flag_tesselation_modifier.pivot += glm::vec2(0.05f * oly::TIME.delta<float>());
		flag_tesselation_parent.local.rotation -= 0.5f * oly::TIME.delta<float>();
		flag_tesselation_parent.post_set();
		flag_tesselation_parent.flush();

		jumble.on_tick();
		jumble.grass_tilemap.set_local().rotation += oly::TIME.delta<float>() * 0.1f;

		// draw
		render_frame(); // LATER put in context::frame()?
	}
}
