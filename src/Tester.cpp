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
	oly::rendering::Paragraph* test_text = nullptr;

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
			if (test_text)
				ref_text_pos = test_text->get_local().position;
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
		if (test_text)
		{
			test_text->set_local().position = ref_text_pos + screen_to_world_coords(cursor_pos) - screen_to_world_coords(ref_cursor_pos);
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

	oly::rendering::Polygon bkg_rect = oly::context::polygon("bkg rect");

	struct Archetype_PolygonCrop
	{
		oly::rendering::Polygon pentagon1, pentagon2;
		oly::rendering::PolyComposite bordered_quad;

		Archetype_PolygonCrop()
			: pentagon1(oly::context::polygon("pentagon1")),
			pentagon2(oly::context::polygon("pentagon2")),
			bordered_quad(oly::context::poly_composite("bordered quad"))
		{}

		void draw(bool flush_polygons) const
		{
			pentagon1.draw();
			pentagon2.draw();
			bordered_quad.draw();
			if (flush_polygons)
				oly::context::render_polygons();
		}
	} polygon_crop;

	struct Archetype_SpriteMatch
	{
		oly::rendering::Sprite sprite0, sprite2;

		Archetype_SpriteMatch()
			: sprite0(oly::context::sprite("sprite0")),
			sprite2(oly::context::sprite("sprite2"))
		{
		}

		void draw(bool flush_sprites) const
		{
			sprite0.draw();
			sprite2.draw();
			if (flush_sprites)
				oly::context::render_sprites();
		}
	} sprite_match;

	struct Archetype_Ellipses
	{
		oly::rendering::Ellipse ellipse1, ellipse2;

		Archetype_Ellipses()
			: ellipse1(oly::context::ellipse("ellipse1")),
			ellipse2(oly::context::ellipse("ellipse2"))
		{
		}

		void draw(bool flush_ellipses) const
		{
			ellipse1.draw();
			ellipse2.draw();
			if (flush_ellipses)
				oly::context::render_ellipses();
		}
	} ellipses;

	struct Archetype_Diverse
	{
		oly::rendering::Sprite sprite3, sprite4, sprite5;
		oly::rendering::NGon octagon;
		oly::rendering::Sprite sprite1, godot_icon_10_0, knight;
		oly::rendering::PolyComposite concave_shape;
		oly::rendering::SpriteAtlasExtension atlased_knight;
		oly::rendering::TileMap grass_tilemap;
		oly::rendering::Paragraph test_text;

		Archetype_Diverse()
			: sprite3(oly::context::sprite("sprite3")),
			sprite4(oly::context::sprite("sprite4")),
			sprite5(oly::context::sprite("sprite5")),
			octagon(oly::context::ngon("octagon")),
			sprite1(oly::context::sprite("sprite1")),
			godot_icon_10_0(oly::context::sprite("godot icon (10.0)")),
			knight(oly::context::sprite("knight")),
			concave_shape(oly::context::poly_composite("concave shape")),
			atlased_knight(oly::context::atlas_extension("atlased knight")),
			grass_tilemap(oly::context::tilemap("grass tilemap")),
			test_text(oly::context::paragraph("test text"))
		{
			octagon.init(); // TODO make protected in Polygonal and call in constructor of Polygon/PolyComposite/NGon
			concave_shape.init();
		}

		void draw(bool flush_text) const
		{
			sprite3.draw();
			sprite4.draw();
			sprite5.draw();
			oly::context::render_sprites();
			octagon.draw();
			oly::context::render_polygons();
			sprite1.draw();
			godot_icon_10_0.draw();
			knight.draw();
			oly::context::render_sprites();
			concave_shape.draw();
			oly::context::render_polygons();
			atlased_knight.sprite.draw();
			grass_tilemap.draw();
			oly::context::render_sprites();
			test_text.draw();
			if (flush_text)
				oly::context::render_text();
		}
	} diverse;

	pc.test_text = &diverse.test_text;

	for (auto& tp : diverse.concave_shape.composite)
	{
		tp.polygon.colors[0].r = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].g = (float)rand() / RAND_MAX;
		tp.polygon.colors[0].b = (float)rand() / RAND_MAX;
	}
	diverse.concave_shape.send_polygon();

	auto flag_texture = oly::context::load_texture("textures/flag.png");

	// LATER begin play on initial actors here

	glEnable(GL_BLEND);

	auto render_frame = [&]() {
		bkg_rect.draw();
		oly::context::render_polygons();
		oly::stencil::begin();
		oly::stencil::enable_drawing();
		glClear(GL_STENCIL_BUFFER_BIT); // must be called after enabling stencil drawing
		oly::stencil::draw::replace();
		polygon_crop.draw(true);
		oly::stencil::disable_drawing();
		oly::stencil::crop::match();
		sprite_match.draw(true);
		oly::stencil::end();
		ellipses.draw(true);
		for (const auto& sprite : flag_tesselation)
			sprite.draw();
		oly::context::render_sprites();
		diverse.draw(true);
		};


	oly::context::attach_standard_window_resize(render_frame);

	while (oly::context::frame())
	{
		// logic update

		diverse.octagon.base.fill_colors[0].r = fmod(oly::TIME.now<float>(), 1.0f);
		diverse.octagon.base.fill_colors[0].b = fmod(oly::TIME.now<float>(), 1.0f);
		diverse.octagon.base.border_width = fmod(oly::TIME.now<float>() * 0.05f, 0.1f);
		diverse.octagon.base.points[6].x = fmod(oly::TIME.now<float>(), 0.6f) - 0.3f;
		diverse.octagon.send_polygon();

		diverse.concave_shape.set_local().rotation += 0.5f * oly::TIME.delta<float>();

		diverse.sprite1.set_local().rotation = oly::TIME.now<float>();
		sprite_match.sprite2.transformer.get_modifier<oly::ShearTransformModifier2D>().shearing.x += 0.5f * oly::TIME.delta<float>();
		sprite_match.sprite2.transformer.post_set();
		
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

		diverse.atlased_knight.on_tick();

		diverse.grass_tilemap.set_local().rotation += oly::TIME.delta<float>() * 0.1f;

		// draw
		render_frame();
	}
}
