#include "Olympian.h"

#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/SAT.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "physics/collision/debugging/CollisionView.h"
#include "physics/collision/objects/Capsule.h"
#include "physics/collision/objects/Combinations.h"
#include "physics/collision/objects/Polygon.h"
#include "physics/collision/scene/CollisionDispatcher.h"

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
	// TODO Log engine initialization/terminatation steps.
	oly::context::Context oly_context("../../../res/context.toml");

	PlayerController pc;
	oly::get_platform().bind_signal("jump", &PlayerController::jump, pc);
	oly::get_platform().bind_signal("click", &PlayerController::click, pc);
	oly::get_platform().bind_signal("drag", &PlayerController::drag, pc);
	oly::get_platform().bind_signal("zoom camera", &PlayerController::zoom_camera, pc);

	KeyHandler key_handler;
	key_handler.attach(&oly::get_platform().window().handlers.key);

	oly::Transformer2D flag_tesselation_parent;
	flag_tesselation_parent.set_modifier() = std::make_unique<oly::PivotShearTransformModifier2D>();
	flag_tesselation_parent.set_local().position.y = -100;
	auto& flag_tesselation_modifier = flag_tesselation_parent.ref_modifier<oly::PivotShearTransformModifier2D>();
	flag_tesselation_modifier = { { 0.0f, 0.0f }, { 400, 320 }, { 0, 1 } };
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

	oly::col2d::CollisionDispatcher collision_dispatcher;
	collision_dispatcher.add_tree(oly::math::Rect2D{ .x1 = -10'000, .x2 = 10'000, .y1 = -10'000, .y2 = 10'000 });

	//oly::col2d::AABB block{ .x1 = -300.0f, .x2 = 100.0f, .y1 = -400.0f, .y2 = 500.0f };
	//oly::col2d::OBB block{ .center = { -100.0f, 50.0f }, .width = 400.0f, .height = 600.0f, .rotation = glm::pi<float>() / 8 };
	//oly::col2d::TPrimitive block(oly::col2d::Circle({ -100.0f, 50.0f }, 200.0f));
	//oly::col2d::TPrimitive block = { oly::col2d::AABB{.x1 = -200.0f, .x2 = 50.0f, .y1 = -200.0f, .y2 = 300.0f } };
	//oly::col2d::TPrimitive block = { oly::col2d::OBB{.center = { -100.0f, 50.0f }, .width = 400.0f, .height = 600.0f, .rotation = -glm::pi<float>() / 6 } };
	//oly::col2d::TPrimitive block(oly::col2d::element(oly::col2d::KDOP3({ -300.0f, -100.0f, -100.0f }, { 100.0f, 100.0f, 100.0f })));

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
	block.handles.attach(collision_dispatcher.get_tree());
	block.set_local().position.y = -100.0f;
	block.set_local().scale.x = 2.0f;
	block.set_local().rotation = glm::pi<float>() / 8;

	//oly::col2d::TPrimitive player(oly::col2d::Circle({}, 50.0f));
	//oly::col2d::TPrimitive player(oly::col2d::AABB{ .x1 = -50.0f, .x2 = 50.0f, .y1 = -50.0f, .y2 = 50.0f });
	oly::col2d::PolygonCollision star;
	const int num_star_points = 5;
	const float outer_star_radius = 50.0f;
	const float inner_star_radius = 15.0f;
	for (int i = 0; i < num_star_points; ++i)
	{
		star.concave_polygon.push_back(inner_star_radius * glm::vec2{ glm::cos((i - 0.25f) * glm::two_pi<float>() / num_star_points), glm::sin((i - 0.25f) * glm::two_pi<float>() / num_star_points) });
		star.concave_polygon.push_back(outer_star_radius * glm::vec2{ glm::cos((i + 0.25f) * glm::two_pi<float>() / num_star_points), glm::sin((i + 0.25f) * glm::two_pi<float>() / num_star_points) });
	}

	oly::col2d::Collider player(star.as_convex_tcompound());
	player.handles.attach(collision_dispatcher.get_tree());
	//oly::col2d::TCompound player = star.as_convex_tcompound();
	//oly::col2d::TBVH<oly::col2d::AABB> player = star.as_convex_tbvh<oly::col2d::AABB>();
	//oly::col2d::TBVH<oly::col2d::OBB> player = star.as_convex_tbvh<oly::col2d::OBB>();
	//oly::col2d::TBVH<oly::col2d::KDOP5> player = star.as_convex_tbvh<oly::col2d::KDOP5>();
	//player.set_heuristic(oly::col2d::Heuristic::MIN_Y_MIN_X);
	//auto player = oly::col2d::Capsule{ .center = {}, .obb_width = 50.0f, .obb_height = 50.0f, .rotation = 0.1f }.tcompound();
	player.set_local().scale.y = 1.2f;
	player.set_local().rotation = glm::pi<float>() / 4;
	
	oly::col2d::Ray ray{ .origin = { -400.0f, -400.0f }, .direction = oly::UnitVector2D(glm::pi<float>() * 0.25f), .clip = 250.0f };

	struct ObstacleCollisionController : public oly::col2d::CollisionController
	{
		OLY_COLLISION_CONTROLLER_HEADER(ObstacleCollisionController);

	public:
		oly::ConstSoftReference<oly::col2d::Collider> player;
		oly::col2d::Collider obstacle1, obstacle2, obstacle3, obstacle4, obstacle5;

		ObstacleCollisionController(const oly::ConstSoftReference<oly::col2d::Collider>& player, oly::col2d::CollisionDispatcher& dispatcher)
			: player(player)
		{
			oly::col2d::Capsule _capsule{ .center = { -400.0f, -400.0f }, .obb_width = 200.0f, .obb_height = 100.0f, .rotation = -0.5f * glm::pi<float>() };
			oly::col2d::TCompound capsule = _capsule.tcompound();
			obstacle1.emplace(capsule);
			obstacle1.handles.attach(dispatcher.get_tree());
			
			capsule.set_local().position.y += 200.0f;
			obstacle2.emplace(capsule);
			obstacle2.handles.attach(dispatcher.get_tree());
			
			capsule.set_local().position.y += 200.0f;
			obstacle3.emplace(capsule);
			obstacle3.handles.attach(dispatcher.get_tree());
			
			capsule.set_local().position.y += 200.0f;
			obstacle4.emplace(capsule);
			obstacle4.handles.attach(dispatcher.get_tree());
			
			capsule.set_local().position.y += 200.0f;
			obstacle5.emplace(capsule);
			obstacle5.handles.attach(dispatcher.get_tree());

			dispatcher.register_handler(obstacle1.cref(), &ObstacleCollisionController::walk_on, cref());
			dispatcher.register_handler(obstacle2.cref(), &ObstacleCollisionController::walk_on, cref());
			dispatcher.register_handler(obstacle3.cref(), &ObstacleCollisionController::walk_on, cref());
			dispatcher.register_handler(obstacle4.cref(), &ObstacleCollisionController::walk_on, cref());
			dispatcher.register_handler(obstacle5.cref(), &ObstacleCollisionController::walk_on, cref());
		}

		void walk_on(const oly::col2d::OverlapEventData& data) const
		{
			if (data.passive_collider == player)
			{
				if (data.active_collider.get() == &obstacle1)
					oly::LOG << "obstacle 1 : " << data.phase << oly::LOG.nl;
				else if (data.active_collider.get() == &obstacle2)
					oly::LOG << "obstacle 2 : " << data.phase << oly::LOG.nl;
				else if (data.active_collider.get() == &obstacle3)
					oly::LOG << "obstacle 3 : " << data.phase << oly::LOG.nl;
				else if (data.active_collider.get() == &obstacle4)
					oly::LOG << "obstacle 4 : " << data.phase << oly::LOG.nl;
				else if (data.active_collider.get() == &obstacle5)
					oly::LOG << "obstacle 5 : " << data.phase << oly::LOG.nl;
			}
		}
	} obstacle_controller(player.cref(), collision_dispatcher);

	//oly::col2d::RectCast rect_cast{ .ray = oly::col2d::Ray{ .origin = {}, .direction = oly::UnitVector2D(-0.25f * glm::pi<float>()), .clip = 200.0f }, .width = 25.0f, .depth = 15.0f};
	//oly::col2d::RectCast rect_cast{ .ray = oly::col2d::Ray{ .origin = {}, .direction = oly::UnitVector2D(-0.25f * glm::pi<float>()), .clip = 0.0f }, .width = 25.0f, .depth = 15.0f};
	oly::col2d::CircleCast circle_cast{ .ray = oly::col2d::Ray{ .origin = {}, .direction = oly::UnitVector2D(-0.25f * glm::pi<float>()), .clip = 200.0f }, .radius = 25.0f };
	//oly::col2d::CircleCast circle_cast{ .ray = oly::col2d::Ray{ .origin = {}, .direction = oly::UnitVector2D(-0.25f * glm::pi<float>()), .clip = 0.0f }, .radius = 25.0f };

	// LATER anti-aliasing settings
	
	oly::CallbackStateTimer flag_state_timer({ 0.5f, 0.5f }, [flag_texture](size_t state) {
		if (state == 0)
			flag_texture->set_and_use_handle(oly::graphics::samplers::nearest);
		else if (state == 1)
			flag_texture->set_and_use_handle(oly::graphics::samplers::linear);
		oly::context::sync_texture_handle(flag_texture);
		}, false);

	oly::debug::CollisionLayer player_layer;
	oly::debug::CollisionLayer obstacle_layer;
	oly::debug::CollisionLayer ray_layer;
	oly::debug::CollisionLayer impulse_layer;
	oly::debug::CollisionLayer raycast_result_layer;

	oly::debug::CollisionView player_impulse_cv, block_impulse_cv, raycast_result_cv;
	oly::debug::CollisionView player_cv = player.collision_view(oly::colors::YELLOW * oly::colors::alpha(0.8f));
	//oly::debug::CollisionView player_cv = oly::debug::collision_view(player, 0, oly::colors::YELLOW * oly::colors::alpha(0.8f));
	//oly::debug::CollisionView block_cv = oly::debug::collision_view(block, oly::colors::BLUE * oly::colors::alpha(0.8f));
	oly::debug::CollisionView block_cv = block.collision_view(oly::colors::BLUE * oly::colors::alpha(0.8f));
	//oly::debug::CollisionView block_cv = oly::debug::collision_view(capsule, oly::colors::BLUE * oly::colors::alpha(0.8f));
	oly::debug::CollisionView ray_cv = oly::debug::collision_view(ray, oly::colors::WHITE * oly::colors::alpha(0.8f));
	//oly::debug::CollisionView rect_cast_cv = oly::debug::collision_view(rect_cast, oly::colors::GREEN * oly::colors::alpha(0.8f), oly::colors::WHITE * oly::colors::alpha(0.8f));
	oly::debug::CollisionView circle_cast_cv = oly::debug::collision_view(circle_cast, oly::colors::GREEN * oly::colors::alpha(0.8f), oly::colors::WHITE * oly::colors::alpha(0.8f));

	auto cv_obstacle1 = obstacle_controller.obstacle1.collision_view(oly::colors::BLUE * oly::colors::alpha(0.8f));
	obstacle_layer.assign(cv_obstacle1);
	auto cv_obstacle2 = obstacle_controller.obstacle2.collision_view(oly::colors::BLUE * oly::colors::alpha(0.8f));
	obstacle_layer.assign(cv_obstacle2);
	auto cv_obstacle3 = obstacle_controller.obstacle3.collision_view(oly::colors::BLUE * oly::colors::alpha(0.8f));
	obstacle_layer.assign(cv_obstacle3);
	auto cv_obstacle4 = obstacle_controller.obstacle4.collision_view(oly::colors::BLUE * oly::colors::alpha(0.8f));
	obstacle_layer.assign(cv_obstacle4);
	auto cv_obstacle5 = obstacle_controller.obstacle5.collision_view(oly::colors::BLUE * oly::colors::alpha(0.8f));
	obstacle_layer.assign(cv_obstacle5);

	player_cv.assign(player_layer);
	block_cv.assign(obstacle_layer);
	player_impulse_cv.assign(impulse_layer);
	block_impulse_cv.assign(impulse_layer);
	ray_cv.assign(ray_layer);
	raycast_result_cv.assign(raycast_result_layer);
	//rect_cast_cv.assign(ray_layer);
	circle_cast_cv.assign(ray_layer);

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

		obstacle_layer.draw();
		player_layer.draw();
		impulse_layer.draw();
		ray_layer.draw();
		raycast_result_layer.draw();
		oly::debug::render_layers();
		};

	oly::context::set_render_function(oly::make_functor(render_frame));

	while (oly::context::frame())
	{
		collision_dispatcher.poll();

		// logic update

		flag_state_timer.poll();
		
		//collision_dispatcher.emit(player);

		player.set_local().position = oly::context::get_cursor_view_pos();
		//player.set_local().position = { 0.0f, 0.0f };
		//player.set<oly::col2d::TCompound>().set_local().position = { -300.0f, -400.0f };
		//oly::LOG << player.get_local().position << oly::LOG.nl;

		//bool point_hits = oly::col2d::point_hits(block, player.get_local().position);
		//bool player_block_overlap = oly::col2d::overlaps(player, block);
		bool player_block_overlap = player.overlaps(block);
		//bool player_block_overlap = oly::col2d::overlaps(player, capsule);

		//auto collide = oly::col2d::collides(player, block);
		auto collide = player.collides(block);
		//auto collide = oly::col2d::collides(player, capsule);
		//auto contact = oly::col2d::contacts(player, block);
		auto contact = player.contacts(block);
		//auto contact = oly::col2d::contacts(player, capsule);

		//if (collide.overlap)
			//oly::LOG << collide.mtv() << oly::LOG.nl;

		//bool ray_hits = oly::col2d::ray_hits(player, ray);
		bool ray_hits = player.ray_hits(ray);

		if (contact.overlap)
		{
			oly::debug::update_view(block_impulse_cv, contact.passive_feature, oly::colors::WHITE * oly::colors::alpha(0.8f));
			oly::debug::update_view(player_impulse_cv, contact.active_feature, oly::colors::WHITE * oly::colors::alpha(0.8f));
		}
		else
		{
			block_impulse_cv.clear_view();
			player_impulse_cv.clear_view();
		}

		//if (fmod(oly::TIME.now<float>(), 2.0f) < 1.0f)
			//oly::debug::update_view(player_cv, player, (ray_hits ? oly::colors::RED : oly::colors::YELLOW) * oly::colors::alpha(0.8f));
		//else
			//oly::debug::update_view(player_cv, player, 2, (point_hits ? oly::colors::RED : oly::colors::YELLOW) * oly::colors::alpha(0.8f));
		
		//oly::debug::update_view(player_cv, player, (contact.overlap ? oly::colors::RED : oly::colors::YELLOW) * oly::colors::alpha(0.8f));
		//bool special_cast_overlap = oly::col2d::rect_cast_hits(player, rect_cast);
		//bool special_cast_overlap = oly::col2d::circle_cast_hits(player, circle_cast);
		bool special_cast_overlap = player.circle_cast_hits(circle_cast);
		player.update_view(player_cv, (special_cast_overlap ? oly::colors::RED : oly::colors::YELLOW) * oly::colors::alpha(0.8f));

		//oly::debug::update_view_color(block_cv, (contact.overlap ? oly::colors::MAGENTA : oly::colors::BLUE) * oly::colors::alpha(0.8f));
		//oly::debug::update_view_color(block_cv, (point_hits ? oly::colors::MAGENTA : oly::colors::BLUE) * oly::colors::alpha(0.8f));
		oly::debug::update_view_color(block_cv, (player_block_overlap ? oly::colors::MAGENTA : oly::colors::BLUE) * oly::colors::alpha(0.8f));

		//auto raycast_result = oly::col2d::raycast(player, ray);
		auto raycast_result = player.raycast(ray);
		if (raycast_result.hit == decltype(raycast_result.hit)::NO_HIT)
			oly::debug::update_view_color(ray_cv, oly::colors::WHITE * oly::colors::alpha(0.8f));
		else if (raycast_result.hit == decltype(raycast_result.hit)::EMBEDDED_ORIGIN)
			oly::debug::update_view_color(ray_cv, oly::colors::YELLOW * oly::colors::alpha(0.8f));
		else if (raycast_result.hit == decltype(raycast_result.hit)::TRUE_HIT)
			oly::debug::update_view_color(ray_cv, oly::colors::ORANGE * oly::colors::alpha(0.8f));

		oly::debug::update_view(raycast_result_cv, raycast_result, oly::colors::WHITE* oly::colors::alpha(0.8f));

		jumble.nonant_panel.set_width(jumble.nonant_panel.width() - 10.0f * oly::TIME.delta<float>());

		jumble.octagon.base.fill_colors[0].r = fmod(oly::TIME.now<float>(), 1.0f);
		jumble.octagon.base.fill_colors[0].b = fmod(oly::TIME.now<float>(), 1.0f);
		jumble.octagon.base.border_width = fmod(oly::TIME.now<float>() * 0.05f, 0.1f);
		jumble.octagon.base.points[6].x = fmod(oly::TIME.now<float>(), 0.6f) - 0.3f;
		jumble.octagon.send_polygon();

		jumble.concave_shape.set_local().rotation += 0.5f * oly::TIME.delta<float>();

		jumble.sprite1.set_local().rotation = oly::TIME.now<float>();
		sprite_match.sprite2.transformer.ref_modifier<oly::ShearTransformModifier2D>().shearing.x += 0.5f * oly::TIME.delta<float>();
		
		flag_tesselation_modifier.pivot += glm::vec2(0.05f * oly::TIME.delta<float>());
		flag_tesselation_parent.set_local().rotation -= 0.5f * oly::TIME.delta<float>();
		flag_tesselation_parent.flush();

		jumble.on_tick();
		jumble.grass_tilemap.set_local().rotation += oly::TIME.delta<float>() * 0.1f;

		// draw
		render_frame(); // LATER put in context::frame()?
	}
}
