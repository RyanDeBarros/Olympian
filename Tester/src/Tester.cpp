#include <Olympian.h>

#include "ProjectContext.h"

#include <assets/graphics/sprites/Sprites.h>
#include <assets/graphics/shapes/Polygons.h>
#include <assets/graphics/text/Paragraphs.h>

#include "SpriteMatch.h"
#include "Jumble.h"

#include "PlayerController.h"

struct KeyHandler : public oly::EventHandler<oly::input::KeyEventData>
{
	virtual bool consume(const oly::input::KeyEventData& data) override
	{
		if (data.action == GLFW_PRESS)
		{
			if (data.key == GLFW_KEY_A)
				OLY_LOG_INFO(true) << "A" << oly::LOG.endl;
			else if (data.key == GLFW_KEY_B)
				OLY_LOG_INFO(true) << "B" << oly::LOG.endl;
			else
				OLY_LOG_INFO(true) << "?" << oly::LOG.endl;
		}
		return false;
	}
};

struct BKG
{
	oly::rendering::PolygonRef bkg_rect;

	BKG()
	{
		bkg_rect = oly::assets::load_polygon(oly::assets::load_toml("~/assets/BKG.toml")["polygon"]);
	}

	void draw() const
	{
		bkg_rect->draw();
	}
};

struct PixelArtText
{
	oly::rendering::ParagraphRef paragraph;

	PixelArtText()
		: paragraph(oly::assets::load_paragraph(oly::assets::load_toml("~/assets/RichParagraph.toml")["paragraph"]))
	{
	}

	void draw() const
	{
		paragraph->draw();
	}
};

struct TesterRenderPipeline : public oly::IRenderPipeline
{
	oly::rendering::PolygonBatch batch;

	BKG bkg;
	SpriteMatch sprite_match;
	Jumble jumble;
	PixelArtText pixel_art_text;

	std::vector<oly::Sprite> flag_tesselation;
	oly::Transformer2D flag_tesselation_parent;
	oly::PivotTransformModifier2D* flag_tesselation_modifier;

	oly::debug::CollisionLayer player_layer;
	oly::debug::CollisionLayer obstacle_layer;
	oly::debug::CollisionLayer ray_layer;
	oly::debug::CollisionLayer impulse_layer;
	oly::debug::CollisionLayer raycast_result_layer;

	oly::CallbackTimer text_jitter_timer;

	TesterRenderPipeline()
		: text_jitter_timer(0.05f, [this](GLuint) { text_jitter_callback(); })
	{
		bkg.bkg_rect->set_batch(batch);

		flag_tesselation_parent.set_modifier() = oly::Polymorphic<oly::PivotTransformModifier2D>();
		flag_tesselation_parent.set_local().position.y = -100;
		flag_tesselation_modifier = &flag_tesselation_parent.ref_modifier<oly::PivotTransformModifier2D>();
		*flag_tesselation_modifier = { { 0.0f, 0.0f }, { 400, 320 } };
		const int flag_rows = 8, flag_cols = 8;
		flag_tesselation.reserve(flag_rows * flag_cols);
		oly::Sprite flag_instance = oly::assets::load_sprite(oly::assets::load_toml("~/assets/flag instance.toml")["sprite"]);
		for (int i = 0; i < flag_rows * flag_cols; ++i)
		{
			flag_tesselation.push_back(flag_instance);
			flag_tesselation[i].set_local().position = { -flag_tesselation_modifier->size.x * 0.5f + float(i % flag_cols) * flag_tesselation_modifier->size.x / flag_cols,
				flag_tesselation_modifier->size.y * 0.5f - float(i / flag_rows) * flag_tesselation_modifier->size.y / flag_rows };
			flag_tesselation[i].transformer.attach_parent(&flag_tesselation_parent);
		}

		oly::default_camera().transformer.set_modifier() = oly::Polymorphic<oly::ShearTransformModifier2D>();

		glEnable(GL_BLEND);

		// TODO v7 anti-aliasing settings
	}

	void render_frame() const override
	{
		bkg.draw();
		batch->render();

		sprite_match.draw();
		for (const auto& sprite : flag_tesselation)
			sprite.draw();
		jumble.draw();

		obstacle_layer.draw();
		player_layer.draw();
		impulse_layer.draw();
		ray_layer.draw();
		raycast_result_layer.draw();
		
		pixel_art_text.draw();
	}

	void logic_update()
	{
		jumble.nonant_panel->set_width(jumble.nonant_panel->width() - 10.0f * oly::TIME.delta());

		jumble.sprite1->set_local().rotation = oly::TIME.now<float>();
		sprite_match.sprite2->transformer.ref_modifier<oly::ShearTransformModifier2D>().shearing.x += 0.5f * oly::TIME.delta();

		flag_tesselation_modifier->pivot += glm::vec2(0.05f * oly::TIME.delta());
		flag_tesselation_parent.set_local().rotation -= 0.5f * oly::TIME.delta();
		flag_tesselation_parent.flush();

		jumble.on_tick();
		jumble.grass_tilemap->set_local().rotation += oly::TIME.delta() * 0.1f;

		// TODO v6 atlased_knight is not drawing
		// TODO v6 camera invariant works except for paragraphs
		jumble.smol_text->set_camera_invariant(true);

		oly::default_camera().transformer.set_local().position.x += oly::TIME.delta() * 20.0f;
		//oly::default_camera().transformer.set_local().rotation += oly::TIME.delta() * 1.0f;
		//oly::default_camera().transformer.set_local().scale.y += oly::TIME.delta() * 0.4f;
		//oly::default_camera().transformer.ref_modifier<oly::ShearTransformModifier2D>().shearing.x += oly::TIME.delta() * 0.2f;
	}

	void text_jitter_callback()
	{
		jumble.test_text->set_element(2).set_jitter_offset({ oly::Random<float>::range(-5.0f, 5.0f), oly::Random<float>::range(-5.0f, 5.0f) });
	}
};

int main()
{
	oly::ProjectContext context;

	oly::context::collision_dispatcher().add_tree(oly::math::Rect2D{ .x1 = -10'000, .x2 = 10'000, .y1 = -10'000, .y2 = 10'000 });

	TesterRenderPipeline pipeline;
	oly::context::set_render_pipeline(&pipeline);

	PlayerController pc;
	pc.bind("jump", &PlayerController::jump);
	pc.bind("jump", &PlayerController::jump);
	pc.bind("click", &PlayerController::click);
	pc.bind("drag", &PlayerController::drag);
	pc.bind("zoom camera", &PlayerController::zoom_camera);
	pc.bind_mapping("move", &PlayerController::move);

	KeyHandler key_handler;
	key_handler.attach(&oly::context_window().handlers.key);

	pc.test_text = pipeline.jumble.test_text.ref;

	oly::col2d::ConvexHull hull_pts;
	const int _npts = 5;
	for (int i = 0; i < _npts; ++i)
	{
		glm::vec2 p;
		const float radius = 100.0f;
		p.x = radius * glm::cos((float)i * glm::two_pi<float>() / (float)_npts);
		p.y = radius * glm::sin((float)i * glm::two_pi<float>() / (float)_npts);
		hull_pts.set_points().push_back(p);
	}
	oly::col2d::Collider block(oly::col2d::TPrimitive(std::move(hull_pts)));
	block.handles.attach();
	block.set_local().position.y = -100.0f;
	block.set_local().scale.x = 2.0f;
	block.set_local().rotation = glm::pi<float>() / 8;

	oly::col2d::PolygonCollision star;
	const int num_star_points = 5;
	const float outer_star_radius = 50.0f;
	const float inner_star_radius = 15.0f;
	for (int i = 0; i < num_star_points; ++i)
	{
		star.concave_polygon.push_back(inner_star_radius * glm::vec2{ glm::cos((i - 0.25f) * glm::two_pi<float>() / num_star_points), glm::sin((i - 0.25f) * glm::two_pi<float>() / num_star_points) });
		star.concave_polygon.push_back(outer_star_radius * glm::vec2{ glm::cos((i + 0.25f) * glm::two_pi<float>() / num_star_points), glm::sin((i + 0.25f) * glm::two_pi<float>() / num_star_points) });
	}

	oly::physics::KinematicBodyRef player = oly::REF_INIT;
	pc.rigid_body = player;
	player->properties().set_moi_multiplier(2000.0f);
	player->properties().net_linear_acceleration += oly::physics::GRAVITY;
	player->properties().set_mass(15.0f);

	oly::col2d::TPrimitive player_collider(oly::col2d::AABB{ .x1 = -50.0f, .x2 = 50.0f, .y1 = -50.0f, .y2 = 50.0f });
	player->add_collider(player_collider);
	player->collider().layer() |= oly::context::get_collision_layer("player");
	player->collider().mask() |= oly::context::get_collision_mask("obstacle");
	player->set_local().scale.y = 1.2f;
	player->set_local().rotation = glm::pi<float>() / 4;

	//player->properties().angular_snapping.enable = true;
	player->properties().angular_snapping.only_colliding = true;
	player->sub_material()->angular_snapping.set_uniformly_spaced(4);

	oly::col2d::Ray ray{ .origin = { -400.0f, -400.0f }, .direction = oly::UnitVector2D(glm::pi<float>() * 0.25f), .clip = 250.0f };

	oly::col2d::Capsule capsule{ .center = { -400.0f, -400.0f }, .obb_width = 200.0f, .obb_height = 100.0f, .rotation = -0.5f * glm::pi<float>() };
	oly::physics::KinematicBodyRef obstacle0 = oly::REF_INIT;
	obstacle0->add_collider(capsule);
	obstacle0->collider().layer() |= oly::context::get_collision_layer("obstacle");
	obstacle0->collider().mask() |= oly::context::get_collision_mask("player") | oly::context::get_collision_mask("obstacle");
	obstacle0->set_local().position = glm::vec2{ 800.0f, 400.0f };
	obstacle0->properties().net_torque += 300.0f;
	obstacle0->properties().set_moi_multiplier(4000.0f);

	capsule.center.y += 200.0f;
	oly::physics::LinearBodyRef obstacle1 = oly::REF_INIT;
	obstacle1->add_collider(capsule);
	obstacle1->collider().layer() |= oly::context::get_collision_layer("obstacle");
	obstacle1->collider().mask() |= oly::context::get_collision_mask("player");
	obstacle1->properties().set_mass(0.0001f);

	capsule.center.y += 200.0f;
	oly::physics::StaticBodyRef obstacle2 = oly::REF_INIT;
	obstacle2->add_collider(capsule);
	obstacle2->collider().layer() |= oly::context::get_collision_layer("obstacle");
	obstacle2->collider().mask() |= oly::context::get_collision_mask("player");

	capsule.center.y += 200.0f;
	oly::physics::StaticBodyRef obstacle3 = oly::REF_INIT;
	obstacle3->add_collider(capsule);
	obstacle3->collider().layer() |= oly::context::get_collision_layer("obstacle");
	obstacle3->collider().mask() |= oly::context::get_collision_mask("player");

	capsule.center.y += 200.0f;
	oly::physics::StaticBodyRef obstacle4 = oly::REF_INIT;
	obstacle4->add_collider(capsule);
	obstacle4->collider().layer() |= oly::context::get_collision_layer("obstacle");
	obstacle4->collider().mask() |= oly::context::get_collision_mask("player");

	oly::physics::StaticBodyRef ground = oly::REF_INIT;
	ground->add_collider(oly::col2d::TCompound({
		oly::col2d::AABB{.x1 = -10'000.0f, .x2 = 10'000.0f, .y1 = -550.0f, .y2 = -450.0f },
		oly::col2d::AABB{.x1 = 400.0f, .x2 = 10'000.0f, .y1 = -550.0f, .y2 = -250.0f }
		}));
	ground->collider().layer() |= oly::context::get_collision_layer("obstacle");
	ground->collider().mask() |= oly::context::get_collision_mask("player");

	oly::physics::StaticBodyRef semi_solid = oly::REF_INIT;
	semi_solid->add_collider(oly::col2d::AABB{ .x1 = -100.0f, .x2 = 300.0f, .y1 = 300.0f, .y2 = 400.0f });
	semi_solid->collider().layer() |= oly::context::get_collision_layer("obstacle");
	semi_solid->collider().mask() |= oly::context::get_collision_mask("player");
	semi_solid->collider().one_way_blocking = oly::UnitVector2D::UP;

	oly::col2d::CircleCast circle_cast{ .ray = oly::col2d::Ray{ .origin = {}, .direction = oly::UnitVector2D(-0.25f * glm::pi<float>()), .clip = 200.0f }, .radius = 25.0f };

	auto player_cv = player->collision_view(pipeline.player_layer, 0, oly::colors::YELLOW * oly::colors::alpha(0.8f));
	auto block_cv = block.collision_view(pipeline.obstacle_layer, oly::colors::BLUE * oly::colors::alpha(0.8f));
	auto ray_cv = oly::debug::collision_view(pipeline.ray_layer, ray, oly::colors::WHITE * oly::colors::alpha(0.8f));
	auto circle_cast_cv = oly::debug::collision_view(pipeline.ray_layer, circle_cast, oly::colors::GREEN * oly::colors::alpha(0.8f), oly::colors::WHITE * oly::colors::alpha(0.8f));

	auto cv_obstacle0 = obstacle0->collision_view(pipeline.obstacle_layer, 0, oly::debug::STANDARD_BLUE);
	auto cv_obstacle1 = obstacle1->collision_view(pipeline.obstacle_layer, 0, oly::debug::STANDARD_BLUE);
	auto cv_obstacle2 = obstacle2->collision_view(pipeline.obstacle_layer, 0, oly::debug::STANDARD_BLUE);
	auto cv_obstacle3 = obstacle3->collision_view(pipeline.obstacle_layer, 0, oly::debug::STANDARD_BLUE);
	auto cv_obstacle4 = obstacle4->collision_view(pipeline.obstacle_layer, 0, oly::debug::STANDARD_BLUE);

	auto ground_cv = ground->collision_view(pipeline.obstacle_layer, 0, glm::vec4{ 111.0f / 255.0f, 78.0f / 255.0f, 55.0f / 255.0f, 1.0f });

	auto semi_solid_cv = semi_solid->collision_view(pipeline.obstacle_layer, 0, glm::vec4{ 0.0f, 78.0f / 255.0f, 55.0f / 255.0f, 1.0f });

	auto flag_texture = oly::context::load_texture("~/textures/flag.png");
	oly::CallbackTimer flag_sampler_timer({ 0.5f, 0.5f }, [flag_texture](size_t state) mutable {
		if (state == 0)
			flag_texture->set_and_use_handle(oly::graphics::samplers::nearest);
		else if (state == 1)
			flag_texture->set_and_use_handle(oly::graphics::samplers::linear);
		oly::context::sync_texture_handle(flag_texture);
		}, false);

	auto modtex = oly::graphics::textures::mod2x2(
		glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f },
		glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f },
		glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f },
		glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f }
	);
	pipeline.jumble.nonant_panel->set_mod_texture(modtex, { 2, 2 });

	// TODO v7 begin play on initial actors here

	oly::LOG.flush();
	while (oly::context::frame())
	{
		player->update_view(0, player_cv);
		obstacle0->update_view(0, cv_obstacle0);
		obstacle1->update_view(0, cv_obstacle1);
		obstacle2->update_view(0, cv_obstacle2);
		obstacle3->update_view(0, cv_obstacle3);
		obstacle4->update_view(0, cv_obstacle4);
		pipeline.logic_update();
	}
}
