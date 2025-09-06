#include "Olympian.h"

#include "ProjectContext.h"

#include "registries/graphics/primitives/Sprites.h"
#include "registries/graphics/TextureRegistry.h"

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
				OLY_LOG_INFO(true) << "A" << oly::LOG.endl;
			else if (data.key == GLFW_KEY_B)
				OLY_LOG_INFO(true) << "B" << oly::LOG.endl;
			else
				OLY_LOG_INFO(true) << "?" << oly::LOG.endl;
		}
		return false;
	}
};

struct TesterRenderPipeline : public oly::IRenderPipeline
{
	oly::gen::BKG bkg;
	oly::gen::PolygonCrop polygon_crop;
	oly::gen::SpriteMatch sprite_match;
	oly::gen::EllipsePair ellipse_pair;
	oly::gen::Jumble jumble;

	std::vector<oly::Sprite> flag_tesselation;
	oly::Transformer2D flag_tesselation_parent;
	oly::PivotTransformModifier2D* flag_tesselation_modifier;

	oly::debug::CollisionLayer player_layer;
	oly::debug::CollisionLayer obstacle_layer;
	oly::debug::CollisionLayer ray_layer;
	oly::debug::CollisionLayer impulse_layer;
	oly::debug::CollisionLayer raycast_result_layer;

	TesterRenderPipeline()
	{
		for (auto& tp : jumble.concave_shape->composite)
		{
			tp.polygon.colors[0].r = (float)rand() / RAND_MAX;
			tp.polygon.colors[0].g = (float)rand() / RAND_MAX;
			tp.polygon.colors[0].b = (float)rand() / RAND_MAX;
		}
		jumble.concave_shape->send_polygon();

		flag_tesselation_parent.set_modifier() = std::make_unique<oly::PivotTransformModifier2D>();
		flag_tesselation_parent.set_local().position.y = -100;
		flag_tesselation_modifier = &flag_tesselation_parent.ref_modifier<oly::PivotTransformModifier2D>();
		*flag_tesselation_modifier = { { 0.0f, 0.0f }, { 400, 320 } };
		const int flag_rows = 8, flag_cols = 8;
		flag_tesselation.reserve(flag_rows * flag_cols);
		oly::Sprite flag_instance = oly::reg::load_sprite(oly::context::load_toml("assets/sprites/flag instance.toml")["sprite"]);
		for (int i = 0; i < flag_rows * flag_cols; ++i)
		{
			flag_tesselation.push_back(flag_instance);
			flag_tesselation[i].set_local().position = { -flag_tesselation_modifier->size.x * 0.5f + float(i % flag_cols) * flag_tesselation_modifier->size.x / flag_cols,
				flag_tesselation_modifier->size.y * 0.5f - float(i / flag_rows) * flag_tesselation_modifier->size.y / flag_rows };
			flag_tesselation[i].transformer.attach_parent(&flag_tesselation_parent);
		}

		glEnable(GL_BLEND);

		// TODO v5 anti-aliasing settings
	}

	void render_frame() const override
	{
		bkg.draw(true);
		oly::stencil::begin();
		oly::stencil::enable_drawing();
		glClear(GL_STENCIL_BUFFER_BIT); // must be called after enabling stencil drawing
		oly::stencil::draw::replace();
		polygon_crop.draw(true); // TODO v4 automatic internal batch tracking
		oly::stencil::disable_drawing();
		oly::stencil::crop::match();
		sprite_match.draw(true);
		oly::stencil::end();
		ellipse_pair.draw(true);
		for (const auto& sprite : flag_tesselation)
			sprite.draw();
		oly::context::render_sprites();
		jumble.draw(true);

		obstacle_layer.draw();
		player_layer.draw();
		impulse_layer.draw();
		ray_layer.draw();
		raycast_result_layer.draw();
		oly::debug::render_layers();
	}

	void logic_update()
	{
		jumble.nonant_panel->set_width(jumble.nonant_panel->width() - 10.0f * oly::TIME.delta());

		jumble.octagon->base.fill_colors[0].r = fmod(oly::TIME.now<float>(), 1.0f);
		jumble.octagon->base.fill_colors[0].b = fmod(oly::TIME.now<float>(), 1.0f);
		jumble.octagon->base.border_width = fmod(oly::TIME.now<float>() * 0.05f, 0.1f);
		jumble.octagon->base.points[6].x = fmod(oly::TIME.now<float>(), 0.6f) - 0.3f;
		jumble.octagon->send_polygon();

		jumble.concave_shape->set_local().rotation += 0.5f * oly::TIME.delta();

		jumble.sprite1->set_local().rotation = oly::TIME.now<float>();
		sprite_match.sprite2->transformer.ref_modifier<oly::ShearTransformModifier2D>().shearing.x += 0.5f * oly::TIME.delta();

		flag_tesselation_modifier->pivot += glm::vec2(0.05f * oly::TIME.delta());
		flag_tesselation_parent.set_local().rotation -= 0.5f * oly::TIME.delta();
		flag_tesselation_parent.flush();

		jumble.on_tick();
		jumble.grass_tilemap->set_local().rotation += oly::TIME.delta() * 0.1f;
	}
};

enum CollisionLayers
{
	L_NONE = 0,
	L_PLAYER = 0b1,
	L_OBSTACLE = 0b10
};

enum CollisionMasks
{
	M_NONE = 0,
	M_PLAYER = 0b1,
	M_OBSTACLE = 0b10
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

	pc.test_text = pipeline.jumble.test_text;

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
	player->properties().set_mass(50.0f);

	oly::col2d::TPrimitive player_collider(oly::col2d::AABB{ .x1 = -50.0f, .x2 = 50.0f, .y1 = -50.0f, .y2 = 50.0f });
	player->add_collider(player_collider);
	player->collider().layer() |= CollisionLayers::L_PLAYER;
	player->collider().mask() |= CollisionMasks::M_OBSTACLE;
	player->set_local().scale.y = 1.2f;
	player->set_local().rotation = glm::pi<float>() / 4;

	player->sub_material()->angular_snapping.set_uniformly_spaced(4);
	player->properties().set_mass(10.0f);

	oly::col2d::Ray ray{ .origin = { -400.0f, -400.0f }, .direction = oly::UnitVector2D(glm::pi<float>() * 0.25f), .clip = 250.0f };

	oly::col2d::Capsule capsule{ .center = { -400.0f, -400.0f }, .obb_width = 200.0f, .obb_height = 100.0f, .rotation = -0.5f * glm::pi<float>() };
	oly::physics::KinematicBodyRef obstacle0 = oly::REF_INIT;
	obstacle0->add_collider(capsule);
	obstacle0->collider().layer() |= CollisionLayers::L_OBSTACLE;
	obstacle0->collider().mask() |= CollisionMasks::M_PLAYER | CollisionMasks::M_OBSTACLE;
	obstacle0->collider().set_local().position = -capsule.center;
	obstacle0->set_transformer().set_modifier() = std::make_unique<oly::OffsetTransformModifier2D>(capsule.center);
	obstacle0->set_local().position = glm::vec2{ 800.0f, 400.0f };
	obstacle0->properties().net_torque += 300.0f;
	obstacle0->properties().set_moi_multiplier(4000.0f);

	capsule.center.y += 200.0f;
	oly::physics::LinearBodyRef obstacle1 = oly::REF_INIT;
	obstacle1->add_collider(capsule);
	obstacle1->collider().layer() |= CollisionLayers::L_OBSTACLE;
	obstacle1->collider().mask() |= CollisionMasks::M_PLAYER;
	obstacle1->properties().set_mass(0.0001f);

	capsule.center.y += 200.0f;
	oly::physics::StaticBodyRef obstacle2 = oly::REF_INIT;
	obstacle2->add_collider(capsule);
	obstacle2->collider().layer() |= CollisionLayers::L_OBSTACLE;
	obstacle2->collider().mask() |= CollisionMasks::M_PLAYER;

	capsule.center.y += 200.0f;
	oly::physics::StaticBodyRef obstacle3 = oly::REF_INIT;
	obstacle3->add_collider(capsule);
	obstacle3->collider().layer() |= CollisionLayers::L_OBSTACLE;
	obstacle3->collider().mask() |= CollisionMasks::M_PLAYER;

	capsule.center.y += 200.0f;
	oly::physics::StaticBodyRef obstacle4 = oly::REF_INIT;
	obstacle4->add_collider(capsule);
	obstacle4->collider().layer() |= CollisionLayers::L_OBSTACLE;
	obstacle4->collider().mask() |= CollisionMasks::M_PLAYER;

	oly::physics::StaticBodyRef ground = oly::REF_INIT;
	ground->add_collider(oly::col2d::TCompound({
		oly::col2d::AABB{.x1 = -10'000.0f, .x2 = 10'000.0f, .y1 = -550.0f, .y2 = -450.0f },
		oly::col2d::AABB{.x1 = 400.0f, .x2 = 10'000.0f, .y1 = -550.0f, .y2 = -250.0f }
		}));
	ground->collider().layer() |= CollisionLayers::L_OBSTACLE;
	ground->collider().mask() |= CollisionMasks::M_PLAYER;

	oly::col2d::CircleCast circle_cast{ .ray = oly::col2d::Ray{ .origin = {}, .direction = oly::UnitVector2D(-0.25f * glm::pi<float>()), .clip = 200.0f }, .radius = 25.0f };

	oly::debug::CollisionView player_impulse_cv, block_impulse_cv, raycast_result_cv;
	oly::debug::CollisionView player_cv = player->collision_view(0, oly::colors::YELLOW * oly::colors::alpha(0.8f));
	oly::debug::CollisionView block_cv = block.collision_view(oly::colors::BLUE * oly::colors::alpha(0.8f));
	oly::debug::CollisionView ray_cv = oly::debug::collision_view(ray, oly::colors::WHITE * oly::colors::alpha(0.8f));
	oly::debug::CollisionView circle_cast_cv = oly::debug::collision_view(circle_cast, oly::colors::GREEN * oly::colors::alpha(0.8f), oly::colors::WHITE * oly::colors::alpha(0.8f));

	auto cv_obstacle0 = obstacle0->collision_view(0, oly::debug::STANDARD_BLUE);
	pipeline.obstacle_layer.assign(cv_obstacle0);
	auto cv_obstacle1 = obstacle1->collision_view(0, oly::debug::STANDARD_BLUE);
	pipeline.obstacle_layer.assign(cv_obstacle1);
	auto cv_obstacle2 = obstacle2->collision_view(0, oly::debug::STANDARD_BLUE);
	pipeline.obstacle_layer.assign(cv_obstacle2);
	auto cv_obstacle3 = obstacle3->collision_view(0, oly::debug::STANDARD_BLUE);
	pipeline.obstacle_layer.assign(cv_obstacle3);
	auto cv_obstacle4 = obstacle4->collision_view(0, oly::debug::STANDARD_BLUE);
	pipeline.obstacle_layer.assign(cv_obstacle4);

	auto ground_cv = ground->collision_view(0, glm::vec4{ 111.0f / 255.0f, 78.0f / 255.0f, 55.0f / 255.0f, 1.0f });
	ground_cv.assign(pipeline.obstacle_layer);

	player_cv.assign(pipeline.player_layer);
	block_cv.assign(pipeline.obstacle_layer);
	player_impulse_cv.assign(pipeline.impulse_layer);
	block_impulse_cv.assign(pipeline.impulse_layer);
	ray_cv.assign(pipeline.ray_layer);
	raycast_result_cv.assign(pipeline.raycast_result_layer);
	circle_cast_cv.assign(pipeline.ray_layer);

	auto flag_texture = oly::context::load_texture("textures/flag.png");
	oly::CallbackTimer flag_sampler_timer({ 0.5f, 0.5f }, [flag_texture](size_t state) mutable {
		if (state == 0)
			flag_texture->set_and_use_handle(oly::graphics::samplers::nearest);
		else if (state == 1)
			flag_texture->set_and_use_handle(oly::graphics::samplers::linear);
		oly::context::sync_texture_handle(flag_texture);
		}, false);

	// TODO v5 begin play on initial actors here

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
