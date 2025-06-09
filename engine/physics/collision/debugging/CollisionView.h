#pragma once

#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"

#include "graphics/primitives/Ellipses.h"
#include "graphics/primitives/Polygons.h"
#include "graphics/extensions/Arrow.h"

#include "core/base/Context.h"

namespace oly::debug
{
	class CollisionView
	{
		using Object = std::variant<rendering::EllipseBatch::EllipseReference, rendering::StaticPolygon, rendering::StaticArrowExtension>;
		struct Empty {};
		enum
		{
			EMPTY,
			SINGLE,
			VECTOR
		};
		using ObjectView = std::variant<Empty, Object, std::vector<Object>>;

		ObjectView obj;

	public:
		CollisionView() : obj(Empty{}) {}
		CollisionView(rendering::EllipseBatch::EllipseReference&& obj) : obj(std::move(obj)) {}
		CollisionView(rendering::StaticPolygon&& obj) : obj(std::move(obj)) {}
		CollisionView(rendering::StaticArrowExtension&& obj) : obj(std::move(obj)) {}

		void draw();
		void clear_view() { obj = Empty{}; }
		void set_view(ObjectView&& obj) { this->obj = std::move(obj); }
		void merge(CollisionView&& other);
	};
	
	extern void render_collision();

	inline CollisionView collision_view(const col2d::Circle& c, glm::vec4 color)
	{
		rendering::EllipseBatch::EllipseReference ellipse;
		ellipse.set_transform() = glm::mat3(col2d::internal::CircleGlobalAccess::get_global(c)) * translation_matrix(c.center);
		auto& dim = ellipse.set_dimension();
		dim.ry = dim.rx = c.radius;
		ellipse.set_color().fill_outer = color;
		return CollisionView(std::move(ellipse));
	}

	inline void update_view(CollisionView& view, const col2d::Circle& c, glm::vec4 color)
	{
		rendering::EllipseBatch::EllipseReference ellipse;
		ellipse.set_transform() = glm::mat3(col2d::internal::CircleGlobalAccess::get_global(c)) * translation_matrix(c.center);
		auto& dim = ellipse.set_dimension();
		dim.ry = dim.rx = c.radius;
		ellipse.set_color().fill_outer = color;
		view.set_view(std::move(ellipse));
	}

	namespace internal
	{
		template<typename Polygon>
		inline debug::CollisionView draw_polygon_collision(const Polygon& points, glm::vec4 color)
		{
			rendering::StaticPolygon polygon;
			polygon.polygon.colors = { color };
			polygon.polygon.points.insert(polygon.polygon.points.end(), points.begin(), points.end());
			polygon.init();
			return CollisionView(std::move(polygon));
		}

		template<typename Polygon>
		inline void update_polygon_view(debug::CollisionView& view, const Polygon& points, glm::vec4 color)
		{
			rendering::StaticPolygon polygon;
			polygon.polygon.colors = { color };
			polygon.polygon.points.insert(polygon.polygon.points.end(), points.begin(), points.end());
			polygon.init();
			view.set_view(std::move(polygon));
		}
	}

	inline CollisionView collision_view(const col2d::AABB& c, glm::vec4 color)
	{
		return internal::draw_polygon_collision(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::AABB& c, glm::vec4 color)
	{
		internal::update_polygon_view(view, c.points(), color);
	}

	inline CollisionView collision_view(const col2d::OBB& c, glm::vec4 color)
	{
		return internal::draw_polygon_collision(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::OBB& c, glm::vec4 color)
	{
		internal::update_polygon_view(view, c.points(), color);
	}

	inline CollisionView collision_view(const col2d::ConvexHull& c, glm::vec4 color)
	{
		return internal::draw_polygon_collision(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::ConvexHull& c, glm::vec4 color)
	{
		internal::update_polygon_view(view, c.points(), color);
	}

	inline CollisionView collision_view(const col2d::CustomKDOP& c, glm::vec4 color)
	{
		return internal::draw_polygon_collision(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::CustomKDOP& c, glm::vec4 color)
	{
		internal::update_polygon_view(view, c.points(), color);
	}

	template<size_t K_half, std::array<UnitVector2D, K_half> Axes>
	inline CollisionView collision_view(const col2d::CustomKDOPShape<K_half, Axes>& c, glm::vec4 color)
	{
		return internal::draw_polygon_collision(c.points(), color);
	}

	template<size_t K_half, std::array<UnitVector2D, K_half> Axes>
	inline void update_view(CollisionView& view, const col2d::CustomKDOPShape<K_half, Axes>& c, glm::vec4 color)
	{
		internal::update_polygon_view(view, c.points(), color);
	}

	template<size_t K_half>
	inline CollisionView collision_view(const col2d::KDOP<K_half>& c, glm::vec4 color)
	{
		return internal::draw_polygon_collision(c.points(), color);
	}

	template<size_t K_half>
	inline void update_view(CollisionView& view, const col2d::KDOP<K_half>& c, glm::vec4 color)
	{
		internal::update_polygon_view(view, c.points(), color);
	}

	inline CollisionView collision_view(const col2d::Element& c, glm::vec4 color)
	{
		return std::visit([color](auto&& e) { return collision_view(e, color); }, c);
	}

	inline void update_view(CollisionView& view, const col2d::Element& c, glm::vec4 color)
	{
		std::visit([&view, color](auto&& e) { update_view(view, e, color); }, c);
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

	inline CollisionView collision_view(const col2d::Compound& c, glm::vec4 color)
	{
		CollisionView view = collision_view(c.elements[0], color);
		for (size_t i = 1; i < c.elements.size(); ++i)
			view.merge(collision_view(c.elements[i], color));
		return view;
	}

	inline void update_view(CollisionView& view, const col2d::Compound& c, glm::vec4 color)
	{
		update_view(view, c.elements[0], color);
		for (size_t i = 1; i < c.elements.size(); ++i)
			view.merge(collision_view(c.elements[i], color));
	}

	inline CollisionView collision_view(const col2d::TCompound& c, glm::vec4 color)
	{
		CollisionView view = collision_view(c.get_baked()[0], color);
		for (size_t i = 1; i < c.get_baked().size(); ++i)
			view.merge(collision_view(c.get_baked()[i], color));
		return view;
	}

	inline void update_view(CollisionView& view, const col2d::TCompound& c, glm::vec4 color)
	{
		update_view(view, c.get_baked()[0], color);
		for (size_t i = 1; i < c.get_baked().size(); ++i)
			view.merge(collision_view(c.get_baked()[i], color));
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::BVH<Shape>& c, glm::vec4 color)
	{
		CollisionView view = collision_view(c.get_elements()[0], color);
		for (size_t i = 1; i < c.get_elements().size(); ++i)
			view.merge(collision_view(c.get_elements()[i], color));
		return view;
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::BVH<Shape>& c, glm::vec4 color)
	{
		update_view(view, c.get_elements()[0], color);
		for (size_t i = 1; i < c.get_elements().size(); ++i)
			view.merge(collision_view(c.get_elements()[i], color));
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::TBVH<Shape>& c, glm::vec4 color)
	{
		CollisionView view = collision_view(c.get_elements()[0], color);
		for (size_t i = 1; i < c.get_elements().size(); ++i)
			view.merge(collision_view(c.get_elements()[i], color));
		return view;
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::TBVH<Shape>& c, glm::vec4 color)
	{
		update_view(view, c.get_elements()[0], color);
		for (size_t i = 1; i < c.get_elements().size(); ++i)
			view.merge(collision_view(c.get_elements()[i], color));
	}

	inline CollisionView collision_view(const col2d::ContactResult::Feature& feature, glm::vec4 color)
	{
		rendering::StaticArrowExtension impulse;
		impulse.set_color(color);
		impulse.adjust_standard_head_for_width(6.0f);
		impulse.set_start() = feature.position;
		impulse.set_end() = feature.position + feature.impulse;
		return CollisionView(std::move(impulse));
	}

	inline void update_view(CollisionView& view, const col2d::ContactResult::Feature& feature, glm::vec4 color)
	{
		rendering::StaticArrowExtension impulse;
		impulse.set_color(color);
		impulse.adjust_standard_head_for_width(6.0f);
		impulse.set_start() = feature.position;
		impulse.set_end() = feature.position + feature.impulse;
		view.set_view(std::move(impulse));
	}
}
