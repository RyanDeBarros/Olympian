#pragma once

#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"
#include "physics/collision/methods/SpecialCasting.h"

#include "graphics/primitives/Sprites.h"
#include "graphics/primitives/Ellipses.h"
#include "graphics/primitives/Polygons.h"
#include "graphics/extensions/Arrow.h"
#include "graphics/backend/basic/Framebuffers.h"

#include "core/platform/Input.h"

namespace oly::debug
{
	enum CollisionObjectType
	{
		ELLIPSE,
		POLYGON,
		ARROW
	};
	using CollisionObject = std::variant<rendering::EllipseBatch::EllipseReference, rendering::StaticPolygon, rendering::StaticArrowExtension>;
	struct EmptyCollision {};
	enum CollisionObjectViewType
	{
		EMPTY,
		SINGLE,
		GROUP
	};
	using CollisionObjectGroup = std::vector<CollisionObject>;
	using CollisionObjectView = std::variant<EmptyCollision, CollisionObject, CollisionObjectGroup>;

	class CollisionLayer;
	class CollisionView
	{
		friend class CollisionLayer;

		CollisionObjectView obj;
		std::unordered_set<CollisionLayer*> layers;

	public:
		CollisionView() : obj(EmptyCollision{}) {}
		CollisionView(rendering::EllipseBatch::EllipseReference&& obj) : obj(std::move(obj)) {}
		CollisionView(rendering::StaticPolygon&& obj) : obj(std::move(obj)) {}
		CollisionView(rendering::StaticArrowExtension&& obj) : obj(std::move(obj)) {}
		CollisionView(const CollisionView&);
		CollisionView(CollisionView&&) noexcept;
		~CollisionView();
		CollisionView& operator=(const CollisionView&);
		CollisionView& operator=(CollisionView&&) noexcept;

	private:
		enum class Batch
		{
			NONE,
			POLYGON,
			ELLIPSE
		};
		void draw(Batch& current_batch) const;

	public:
		void clear_view();
		void set_view(CollisionObjectView&& obj);
		void merge(CollisionView&& other);

		void assign(CollisionLayer& layer);
		void unassign(CollisionLayer& layer);

		const CollisionObjectView& get_view() const { return obj; }
		CollisionObjectView& get_view() { return obj; }
		void view_changed() const;
	};

	class CollisionLayer
	{
		friend class CollisionView;

		rendering::StaticSprite sprite;
		graphics::Framebuffer framebuffer;
		graphics::BindlessTextureRes texture;
		glm::ivec2 dimensions;
		mutable bool dirty_views = false;
		std::unordered_set<CollisionView*> collision_views;

		struct WindowResizeHandler : public EventHandler<input::WindowResizeEventData>
		{
			CollisionLayer* layer = nullptr;

			WindowResizeHandler();

			bool consume(const input::WindowResizeEventData& data) override;
		} window_resize_handler;
		friend struct WindowResizeHandler;

	public:
		CollisionLayer();
		CollisionLayer(const CollisionLayer&);
		CollisionLayer(CollisionLayer&&) noexcept;
		~CollisionLayer();
		CollisionLayer& operator=(const CollisionLayer&);
		CollisionLayer& operator=(CollisionLayer&&) noexcept;

	private:
		void write_texture() const;
		void set_sprite_scale(glm::vec2 scale);

	public:
		void draw() const;

		void assign(CollisionView& view);
		void unassign(CollisionView& view);

		void regen_to_current_resolution();

	private:
		void setup_texture();
	};
	
	extern void render_layers();

	inline void update_view_color(CollisionView& view, glm::vec4 color)
	{
		CollisionObjectView& obj = view.get_view();

		static const auto update_color = [](CollisionObject& obj, glm::vec4 color) {
			std::visit([color](auto&& obj) {
				if constexpr (visiting_class_is<decltype(obj), rendering::EllipseBatch::EllipseReference>)
					obj.set_color().fill_outer = color;
				else if constexpr (visiting_class_is<decltype(obj), rendering::StaticPolygon>)
				{
					obj.polygon.colors = { color };
					obj.send_colors_only();
				}
				else if constexpr (visiting_class_is<decltype(obj), rendering::StaticArrowExtension>)
					obj.set_color(color);
				}, obj);
			};

		bool view_changed = std::visit([color](auto&& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), CollisionObject>)
			{
				update_color(obj, color);
				return true;
			}
			else if constexpr (visiting_class_is<decltype(obj), CollisionObjectGroup>)
			{
				for (CollisionObject& subobj : obj)
					update_color(subobj, color);
				return !obj.empty();
			}
			else
				return false;
			}, obj);

		if (view_changed)
			view.view_changed();
	}

	inline CollisionView collision_view(const col2d::Circle& c, glm::vec4 color)
	{
		rendering::EllipseBatch::EllipseReference ellipse;
		ellipse.set_transform() = augment(col2d::internal::CircleGlobalAccess::get_global(c), col2d::internal::CircleGlobalAccess::get_global_offset(c)) * translation_matrix(c.center);
		auto& dim = ellipse.set_dimension();
		dim.ry = dim.rx = c.radius;
		ellipse.set_color().fill_outer = color;
		return CollisionView(std::move(ellipse));
	}

	// TODO update_view without color setting
	inline void update_view(CollisionView& view, const col2d::Circle& c, glm::vec4 color)
	{
		CollisionObjectView& obj = view.get_view();
		bool modify = std::visit([](auto&& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), CollisionObject>)
			{
				return std::visit([](auto&& obj) -> bool {
					if constexpr (visiting_class_is<decltype(obj), rendering::EllipseBatch::EllipseReference>)
						return true;
					else
						return false;
					}, obj);
			}
			else
				return false;
			}, obj);

		if (modify)
		{
			rendering::EllipseBatch::EllipseReference& ellipse = std::get<CollisionObjectType::ELLIPSE>(std::get<CollisionObjectViewType::SINGLE>(obj));
			ellipse.set_transform() = augment(col2d::internal::CircleGlobalAccess::get_global(c), col2d::internal::CircleGlobalAccess::get_global_offset(c)) * translation_matrix(c.center);
			auto& dim = ellipse.set_dimension();
			dim.ry = dim.rx = c.radius;
			ellipse.set_color().fill_outer = color;
			view.view_changed();
		}
		else
		{
			rendering::EllipseBatch::EllipseReference ellipse;
			ellipse.set_transform() = augment(col2d::internal::CircleGlobalAccess::get_global(c), col2d::internal::CircleGlobalAccess::get_global_offset(c)) * translation_matrix(c.center);
			auto& dim = ellipse.set_dimension();
			dim.ry = dim.rx = c.radius;
			ellipse.set_color().fill_outer = color;
			view.set_view(std::move(ellipse));
		}
	}

	namespace internal
	{
		template<typename Polygon>
		inline debug::CollisionView polygon_collision_view(const Polygon& points, glm::vec4 color)
		{
			if (points.size() < 3)
				return CollisionView();

			rendering::StaticPolygon polygon;
			polygon.polygon.colors = { color };
			polygon.polygon.points.insert(polygon.polygon.points.end(), points.begin(), points.end());
			polygon.init();
			return CollisionView(std::move(polygon));
		}

		template<typename Polygon>
		inline void polygon_update_view(debug::CollisionView& view, const Polygon& points, glm::vec4 color)
		{
			if (points.size() < 3)
			{
				view.clear_view();
				return;
			}

			CollisionObjectView& obj = view.get_view();
			bool modify = std::visit([](auto&& obj) -> bool {
				if constexpr (visiting_class_is<decltype(obj), CollisionObject>)
				{
					return std::visit([](auto&& obj) -> bool {
						if constexpr (visiting_class_is<decltype(obj), rendering::StaticPolygon>)
							return true;
						else
							return false;
						}, obj);
				}
				else
					return false;
				}, obj);

			if (modify)
			{
				rendering::StaticPolygon& polygon = std::get<CollisionObjectType::POLYGON>(std::get<CollisionObjectViewType::SINGLE>(obj));
				polygon.polygon.colors = { color };
				polygon.polygon.points.clear();
				polygon.polygon.points.insert(polygon.polygon.points.end(), points.begin(), points.end());
				polygon.send_polygon();
				view.view_changed();
			}
			else
			{
				rendering::StaticPolygon polygon;
				polygon.polygon.colors = { color };
				polygon.polygon.points.insert(polygon.polygon.points.end(), points.begin(), points.end());
				polygon.init();
				view.set_view(std::move(polygon));
			}
		}
	}

	inline CollisionView collision_view(const col2d::AABB& c, glm::vec4 color)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::AABB& c, glm::vec4 color)
	{
		internal::polygon_update_view(view, c.points(), color);
	}

	inline CollisionView collision_view(const col2d::OBB& c, glm::vec4 color)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::OBB& c, glm::vec4 color)
	{
		internal::polygon_update_view(view, c.points(), color);
	}

	inline CollisionView collision_view(const col2d::ConvexHull& c, glm::vec4 color)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::ConvexHull& c, glm::vec4 color)
	{
		internal::polygon_update_view(view, c.points(), color);
	}

	template<size_t K>
	inline CollisionView collision_view(const col2d::KDOP<K>& c, glm::vec4 color)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	template<size_t K>
	inline void update_view(CollisionView& view, const col2d::KDOP<K>& c, glm::vec4 color)
	{
		internal::polygon_update_view(view, c.points(), color);
	}

	inline CollisionView collision_view(const col2d::Element& c, glm::vec4 color)
	{
		return std::visit([color](auto&& e) { return collision_view(*e, color); }, param(c));
	}

	inline void update_view(CollisionView& view, const col2d::Element& c, glm::vec4 color)
	{
		std::visit([&view, color](auto&& e) { update_view(view, *e, color); }, param(c));
	}

	inline CollisionView collision_view(const col2d::ElementParam& c, glm::vec4 color)
	{
		return std::visit([color](auto&& e) { return collision_view(*e, color); }, c);
	}

	inline void update_view(CollisionView& view, const col2d::ElementParam& c, glm::vec4 color)
	{
		std::visit([&view, color](auto&& e) { update_view(view, *e, color); }, c);
	}

	inline CollisionView collision_view(const col2d::Primitive& c, glm::vec4 color)
	{
		return collision_view(c.element, color);
	}

	inline void update_view(CollisionView& view, const col2d::Primitive& c, glm::vec4 color)
	{
		update_view(view, c.element, color);
	}

	inline CollisionView collision_view(const col2d::TPrimitive& c, glm::vec4 color)
	{
		return collision_view(c.get_baked(), color);
	}

	inline void update_view(CollisionView& view, const col2d::TPrimitive& c, glm::vec4 color)
	{
		update_view(view, c.get_baked(), color);
	}

	template<typename Object>
	inline CollisionView collision_view(const std::vector<Object>& elements, glm::vec4 color)
	{
		if (elements.empty())
			return {};
		CollisionView view = collision_view(elements[0], color);
		for (size_t i = 1; i < elements.size(); ++i)
			view.merge(collision_view(elements[i], color));
		return view;
	}

	template<typename Object>
	inline void update_view(CollisionView& view, const std::vector<Object>& elements, glm::vec4 color)
	{
		if (elements.empty())
			view.clear_view();
		else
		{
			update_view(view, elements[0], color);
			for (size_t i = 1; i < elements.size(); ++i)
				view.merge(collision_view(elements[i], color));
		}
	}

	inline CollisionView collision_view(const col2d::Compound& c, glm::vec4 color)
	{
		return collision_view(c.elements, color);
	}

	inline void update_view(CollisionView& view, const col2d::Compound& c, glm::vec4 color)
	{
		update_view(view, c.elements, color);
	}

	inline CollisionView collision_view(const col2d::TCompound& c, glm::vec4 color)
	{
		return collision_view(c.get_baked(), color);
	}

	inline void update_view(CollisionView& view, const col2d::TCompound& c, glm::vec4 color)
	{
		update_view(view, c.get_baked(), color);
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::BVH<Shape>& c, glm::vec4 color)
	{
		return collision_view(c.get_elements(), color);
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::BVH<Shape>& c, size_t depth, glm::vec4 color)
	{
		CollisionView view;
		auto layer = c.build_layer(depth);
		for (const auto& e : layer)
		{
			std::visit([&view, color] (auto&& e) {
				if constexpr (visiting_class_is<decltype(e), Shape>)
					view.merge(collision_view(*e, color));
				else
					view.merge(collision_view(e, color));
				}, e);
		}
		return view;
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::BVH<Shape>& c, glm::vec4 color)
	{
		update_view(view, c.get_elements(), color);
	}
	
	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::BVH<Shape>& c, size_t depth, glm::vec4 color)
	{
		auto layer = c.build_layer(depth);
		if (layer.empty())
			view.clear_view();
		else
		{
			std::visit([&view, color](auto&& e) {
				if constexpr (visiting_class_is<decltype(e), Shape>)
					update_view(view, *e, color);
				else
					update_view(view, e, color);
				}, layer[0]);
			for (size_t i = 1; i < layer.size(); ++i)
			{
				std::visit([&view, color](auto&& e) {
					if constexpr (visiting_class_is<decltype(e), Shape>)
						view.merge(collision_view(*e, color));
					else
						view.merge(collision_view(e, color));
					}, layer[i]);
			}
		}
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::TBVH<Shape>& c, glm::vec4 color)
	{
		return collision_view(c.get_baked(), color);
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::TBVH<Shape>& c, size_t depth, glm::vec4 color)
	{
		CollisionView view;
		auto layer = c.build_layer(depth);
		for (const auto& e : layer)
		{
			std::visit([&view, color](auto&& e) {
				if constexpr (visiting_class_is<decltype(e), Shape>)
					view.merge(collision_view(*e, color));
				else
					view.merge(collision_view(e, color));
				}, e);
		}
		return view;
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::TBVH<Shape>& c, glm::vec4 color)
	{
		update_view(view, c.get_baked(), color);
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::TBVH<Shape>& c, size_t depth, glm::vec4 color)
	{
		auto layer = c.build_layer(depth);
		if (layer.empty())
			view.clear_view();
		else
		{
			std::visit([&view, color](auto&& e) {
				if constexpr (visiting_class_is<decltype(e), Shape>)
					update_view(view, *e, color);
				else
					update_view(view, e, color);
				}, layer[0]);
			for (size_t i = 1; i < layer.size(); ++i)
			{
				std::visit([&view, color](auto&& e) {
					if constexpr (visiting_class_is<decltype(e), Shape>)
						view.merge(collision_view(*e, color));
					else
						view.merge(collision_view(e, color));
					}, layer[i]);
			}
		}
	}

	inline CollisionView collision_view(const col2d::ContactResult::Feature& feature, glm::vec4 color, float arrow_width = 6.0f)
	{
		rendering::StaticArrowExtension impulse;
		impulse.set_color(color);
		impulse.adjust_standard_head_for_width(arrow_width);
		impulse.set_start() = feature.position;
		impulse.set_end() = feature.position + feature.impulse;
		return CollisionView(std::move(impulse));
	}

	inline void update_view(CollisionView& view, const col2d::ContactResult::Feature& feature, glm::vec4 color, float arrow_width = 6.0f)
	{
		CollisionObjectView& obj = view.get_view();
		bool modify = std::visit([](auto&& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), CollisionObject>)
			{
				return std::visit([](auto&& obj) -> bool {
					if constexpr (visiting_class_is<decltype(obj), rendering::StaticArrowExtension>)
						return true;
					else
						return false;
					}, obj);
			}
			else
				return false;
			}, obj);

		if (modify)
		{
			rendering::StaticArrowExtension& impulse = std::get<CollisionObjectType::ARROW>(std::get<CollisionObjectViewType::SINGLE>(obj));
			impulse.set_color(color);
			impulse.adjust_standard_head_for_width(arrow_width);
			impulse.set_start() = feature.position;
			impulse.set_end() = feature.position + feature.impulse;
			view.view_changed();
		}
		else
		{
			rendering::StaticArrowExtension impulse;
			impulse.set_color(color);
			impulse.adjust_standard_head_for_width(arrow_width);
			impulse.set_start() = feature.position;
			impulse.set_end() = feature.position + feature.impulse;
			view.set_view(std::move(impulse));
		}
	}

	constexpr float INFINITE_RAY_LENGTH = 1'000'000.0f;

	inline CollisionView collision_view(const col2d::Ray& ray, glm::vec4 color, float arrow_width = 6.0f)
	{
		rendering::StaticArrowExtension arrow;
		arrow.set_color(color);
		arrow.adjust_standard_head_for_width(arrow_width);
		arrow.set_start() = ray.origin;
		float clip = ray.clip == 0.0f ? INFINITE_RAY_LENGTH : ray.clip;
		arrow.set_end() = ray.origin + clip * (glm::vec2)ray.direction;
		return CollisionView(std::move(arrow));
	}

	inline void update_view(CollisionView& view, const col2d::Ray& ray, glm::vec4 color, float arrow_width = 6.0f)
	{
		CollisionObjectView& obj = view.get_view();
		bool modify = std::visit([](auto&& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), CollisionObject>)
			{
				return std::visit([](auto&& obj) -> bool {
					if constexpr (visiting_class_is<decltype(obj), rendering::StaticArrowExtension>)
						return true;
					else
						return false;
					}, obj);
			}
			else
				return false;
			}, obj);

		if (modify)
		{
			rendering::StaticArrowExtension& arrow = std::get<CollisionObjectType::ARROW>(std::get<CollisionObjectViewType::SINGLE>(obj));
			arrow.set_color(color);
			arrow.adjust_standard_head_for_width(arrow_width);
			arrow.set_start() = ray.origin;
			arrow.set_end() = ray.clip == 0.0f ? (ray.origin + INFINITE_RAY_LENGTH * (glm::vec2)ray.direction) : ray.origin + ray.clip * (glm::vec2)ray.direction;
			view.view_changed();
		}
		else
		{
			rendering::StaticArrowExtension arrow;
			arrow.set_color(color);
			arrow.adjust_standard_head_for_width(arrow_width);
			arrow.set_start() = ray.origin;
			arrow.set_end() = ray.clip == 0.0f ? (ray.origin + INFINITE_RAY_LENGTH * (glm::vec2)ray.direction) : ray.origin + ray.clip * (glm::vec2)ray.direction;
			view.set_view(std::move(arrow));
		}
	}

	inline CollisionView collision_view(const col2d::RaycastResult& result, glm::vec4 color, float impulse_length = 50.0f, float arrow_width = 6.0f)
	{
		if (result.hit == col2d::RaycastResult::Hit::TRUE_HIT)
		{
			col2d::Ray ray{ .origin = result.contact, .direction = result.normal, .clip = impulse_length };
			return collision_view(ray, color, arrow_width);
		}
		else
			return CollisionView();
	}

	inline void update_view(CollisionView& view, const col2d::RaycastResult& result, glm::vec4 color, float impulse_length = 50.0f, float arrow_width = 6.0f)
	{
		if (result.hit == col2d::RaycastResult::Hit::TRUE_HIT)
		{
			col2d::Ray ray{ .origin = result.contact, .direction = result.normal, .clip = impulse_length };
			update_view(view, ray, color, arrow_width);
		}
		else
			view.clear_view();
	}

	inline CollisionView collision_view(const col2d::RectCast& cast, glm::vec4 color, glm::vec4 arrow_color)
	{
		CollisionView view = collision_view(cast.finite_obb(INFINITE_RAY_LENGTH), color);
		view.merge(collision_view(cast.ray, arrow_color));
		return view;
	}

	inline void update_view(CollisionView& view, const col2d::RectCast& cast, glm::vec4 color, glm::vec4 arrow_color)
	{
		update_view(view, cast.finite_obb(INFINITE_RAY_LENGTH), color);
		view.merge(collision_view(cast.ray, arrow_color));
	}

	inline CollisionView collision_view(const col2d::CircleCast& cast, glm::vec4 color, glm::vec4 arrow_color)
	{
		CollisionView view = collision_view(cast.finite_capsule(INFINITE_RAY_LENGTH).compound(), color);
		view.merge(collision_view(cast.ray, arrow_color));
		return view;
	}

	// TODO update_view can be better specialized. can check if view is vector of #X objects, and individually update them.
	inline void update_view(CollisionView& view, const col2d::CircleCast& cast, glm::vec4 color, glm::vec4 arrow_color)
	{
		update_view(view, cast.finite_capsule(INFINITE_RAY_LENGTH).compound(), color);
		view.merge(collision_view(cast.ray, arrow_color));
	}
}
