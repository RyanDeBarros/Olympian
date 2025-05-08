#include "Olympian.h"

#include "registries/graphics/Textures.h"

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

struct PlayerController : public oly::InputController
{
	bool dragging = false;
	glm::vec2 ref_cursor_pos = {};
	glm::vec2 ref_text_pos = {};

	bool jump(oly::input::Signal signal)
	{
		if (signal.phase == oly::input::Phase::STARTED)
		{
			oly::LOG << "Jump!" << oly::LOG.endl;
			return true;
		}
		return false;
	}

	bool click(oly::input::Signal signal)
	{
		if (signal.phase == oly::input::Phase::STARTED)
		{
			dragging = true;
			double x, y;
			glfwGetCursorPos(oly::get_platform().window(), &x, &y);
			ref_cursor_pos = { (float)x, (float)y };
			if (auto sp = oly::context::ref_paragraph("test text").lock())
				ref_text_pos = sp->get_local().position;
			drag(ref_cursor_pos);
			return true;
		}
		else if (signal.phase == oly::input::Phase::COMPLETED)
		{
			dragging = false;
			return true;
		}
		return false;
	}

	bool drag(oly::input::Signal signal)
	{
		if (dragging)
			return drag(signal.get<glm::vec2>());
		return false;
	}

	bool drag(glm::vec2 cursor_pos)
	{
		if (auto sp = oly::context::ref_paragraph("test text").lock())
		{
			oly::LOG << cursor_pos << oly::LOG.endl;
			sp->set_local().position = ref_text_pos + screen_to_world_coords(cursor_pos) - screen_to_world_coords(ref_cursor_pos);
			return true;
		}
		return false;
	}

	glm::vec2 screen_to_world_coords(glm::vec2 coords)
	{
		return { coords.x - 0.5f * oly::get_platform().window().get_width(), 0.5f * oly::get_platform().window().get_height() - coords.y };
	}

	bool zoom_camera(oly::input::Signal signal)
	{
		switch (signal.phase)
		{
		case oly::input::Phase::STARTED:
			oly::LOG << "STARTED:   " << signal.get<glm::vec2>() << oly::LOG.endl;
			return true;
		case oly::input::Phase::ONGOING:
			oly::LOG << "ONGOING:   " << signal.get<glm::vec2>() << oly::LOG.endl;
			return true;
		case oly::input::Phase::COMPLETED:
			oly::LOG << "COMPLETED: " << signal.get<glm::vec2>() << oly::LOG.endl;
			return true;
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
	for (int i = 0; i < flag_rows * flag_cols; ++i)
	{
		flag_tesselation.push_back(oly::sprite("flag instance"));
		flag_tesselation[i].set_local().position = { -flag_tesselation_modifier.size.x * 0.5f + float(i % flag_cols) * flag_tesselation_modifier.size.x / flag_cols,
			flag_tesselation_modifier.size.y * 0.5f - float(i / flag_rows) * flag_tesselation_modifier.size.y / flag_rows };
		flag_tesselation[i].transformer.attach_parent(&flag_tesselation_parent);
	}

	auto concave_shape = oly::context::ref_poly_composite("concave shape").lock();
	for (auto& tp : concave_shape->composite)
	{
		tp.polygon.colors[0].r = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].g = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].b = (float)rand() / RAND_MAX;
	}
	concave_shape->send_polygon();


	auto bkg_rect = oly::context::ref_polygon("bkg rect").lock();
	auto octagon = oly::context::ref_ngon("octagon").lock();
	auto flag_texture = oly::context::load_texture("textures/flag.png");
	auto atlased_knight = oly::context::ref_atlas_extension("atlased knight").lock();
	auto tilemap = oly::context::ref_tilemap("grass tilemap").lock();
	
	// LATER begin play on initial actors here

	glEnable(GL_BLEND);

	auto render_frame = [&]() {
		bkg_rect->draw();
		oly::context::render_polygons();
		oly::stencil::begin();
		oly::stencil::enable_drawing();
		glClear(GL_STENCIL_BUFFER_BIT); // must be called after enabling stencil drawing
		oly::stencil::draw::replace();
		oly::context::execute_draw_command("polygon crop");
		oly::stencil::disable_drawing();
		oly::stencil::crop::match();
		oly::context::execute_draw_command("sprite match");
		oly::stencil::end();
		oly::context::execute_draw_command("ellipses");
		for (const auto& sprite : flag_tesselation)
			sprite.draw();
		oly::context::render_sprites();
		oly::context::execute_draw_command("sprites, polygons, tilemaps, and text");
		};


	oly::context::attach_standard_window_resize(render_frame);

	while (oly::context::frame())
	{
		// logic update

		octagon->base.fill_colors[0].r = fmod(oly::TIME.now<float>(), 1.0f);
		octagon->base.fill_colors[0].b = fmod(oly::TIME.now<float>(), 1.0f);
		octagon->base.border_width = fmod(oly::TIME.now<float>() * 0.05f, 0.1f);
		octagon->base.points[6].x = fmod(oly::TIME.now<float>(), 0.6f) - 0.3f;
		octagon->send_polygon();

		concave_shape->set_local().rotation += 0.5f * oly::TIME.delta<float>();

		if (auto sprite1 = oly::context::ref_sprite("sprite1").lock())
			sprite1->set_local().rotation = oly::TIME.now<float>();
		if (auto sprite2 = oly::context::ref_sprite("sprite2").lock())
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
				flag_texture->set_and_use_handle(oly::graphics::samplers::nearest);
				oly::context::sync_texture_handle(flag_texture);
			}
		}
		else
		{
			if (fmod(oly::TIME.now<float>(), 1.0f) < 0.5f)
			{
				lin = true;
				flag_texture->set_and_use_handle(oly::graphics::samplers::linear);
				oly::context::sync_texture_handle(flag_texture);
			}
		}

		atlased_knight->on_tick();

		tilemap->set_local().rotation += oly::TIME.delta<float>() * 0.1f;

		// draw
		render_frame();
	}
}
