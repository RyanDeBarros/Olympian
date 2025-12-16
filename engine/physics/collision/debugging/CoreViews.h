#pragma once

#include "physics/collision/debugging/CollisionView.h"

namespace oly::debug
{
	constexpr glm::vec4 STANDARD_BLUE = glm::vec4{ 0.0f, 0.0f, 1.0f, 0.8f };

	inline CollisionObjectView collision_view(const col2d::Circle& c, glm::vec4 color = STANDARD_BLUE)
	{
		rendering::StaticEllipse ellipse;
		ellipse.set_transform(c.compute_transform());
		auto dim = ellipse.get_dimension();
		dim.ry = dim.rx = c.radius;
		ellipse.set_dimension(dim);
		ellipse.set_color(color);
		return ellipse;
	}

	inline void update_view(CollisionView& view, const col2d::Circle& c, size_t view_index = 0)
	{
		if (auto obj = view.get_view(view_index)->safe_get<rendering::StaticEllipse>())
		{
			obj->set_transform(c.compute_transform());
			auto dim = obj->get_dimension();
			dim.ry = dim.rx = c.radius;
			obj->set_dimension(dim);
			view.view_changed();
		}
		else
		{
			rendering::StaticEllipse ellipse = view.get_layer().create_ellipse();
			ellipse.set_transform(c.compute_transform());
			auto dim = ellipse.get_dimension();
			dim.ry = dim.rx = c.radius;
			ellipse.set_dimension(dim);
			view.set_view(std::move(ellipse));
		}
	}

	namespace internal
	{
		template<typename Polygon>
		inline CollisionObjectView polygon_collision_view(const Polygon& points, glm::vec4 color = STANDARD_BLUE)
		{
			if (points.size() < 3)
				return EmptyCollision();

			rendering::StaticPolygon polygon;
			polygon.set_colors() = { color };
			polygon.set_points().insert(polygon.get_points().end(), points.begin(), points.end());
			return polygon;
		}

		template<typename Polygon>
		inline void polygon_update_view(CollisionView& view, const Polygon& points, size_t view_index = 0)
		{
			if (points.size() < 3)
			{
				view.clear_view();
				return;
			}

			if (auto obj = view.get_view(view_index)->safe_get<rendering::StaticPolygon>())
			{
				obj->set_points().clear();
				obj->set_points().insert(obj->get_points().end(), points.begin(), points.end());
				view.view_changed();
			}
			else
			{
				rendering::StaticPolygon polygon = view.get_layer().create_polygon();
				polygon.set_colors() = { glm::vec4{ 0.0f, 0.0f, 1.0f, 0.8f } };
				polygon.set_points().insert(polygon.get_points().end(), points.begin(), points.end());
				view.set_view(std::move(polygon));
			}
		}
	}

	inline CollisionObjectView collision_view(const col2d::AABB& c, glm::vec4 color = STANDARD_BLUE)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::AABB& c, size_t view_index = 0)
	{
		internal::polygon_update_view(view, c.points(), view_index);
	}

	inline CollisionObjectView collision_view(const col2d::OBB& c, glm::vec4 color = STANDARD_BLUE)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::OBB& c, size_t view_index = 0)
	{
		internal::polygon_update_view(view, c.points(), view_index);
	}

	inline CollisionObjectView collision_view(const col2d::ConvexHull& c, glm::vec4 color = STANDARD_BLUE)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::ConvexHull& c, size_t view_index = 0)
	{
		internal::polygon_update_view(view, c.points(), view_index);
	}

	template<size_t K>
	inline CollisionObjectView collision_view(const col2d::KDOP<K>& c, glm::vec4 color = STANDARD_BLUE)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	template<size_t K>
	inline void update_view(CollisionView& view, const col2d::KDOP<K>& c, size_t view_index = 0)
	{
		internal::polygon_update_view(view, c.points(), view_index);
	}

	inline CollisionObjectView collision_view(const col2d::Element& c, glm::vec4 color = STANDARD_BLUE)
	{
		return c.variant().visit([color](const auto& e) { return collision_view(*e, color); });
	}

	inline void update_view(CollisionView& view, const col2d::Element& c, size_t view_index = 0)
	{
		c.variant().visit([&view, view_index](const auto& e) { update_view(view, *e, view_index); });
	}

	inline CollisionObjectView collision_view(const col2d::Primitive& c, glm::vec4 color = STANDARD_BLUE)
	{
		return collision_view(c.element, color);
	}

	inline void update_view(CollisionView& view, const col2d::Primitive& c, size_t view_index = 0)
	{
		update_view(view, c.element, view_index);
	}

	inline CollisionObjectView collision_view(const col2d::TPrimitive& c, glm::vec4 color = STANDARD_BLUE)
	{
		return collision_view(c.get_baked(), color);
	}

	inline void update_view(CollisionView& view, const col2d::TPrimitive& c, size_t view_index = 0)
	{
		update_view(view, c.get_baked(), view_index);
	}

	template<typename Object>
	inline CollisionObjectView collision_view(const std::vector<Object>& elements, glm::vec4 color = STANDARD_BLUE)
	{
		if (elements.empty())
			return EmptyCollision();

		CollisionObjectView view = collision_view(elements[0], color);
		for (size_t i = 1; i < elements.size(); ++i)
			view.merge(collision_view(elements[i], color));
		return view;
	}

	template<typename Object>
	inline void update_view(CollisionView& view, const std::vector<Object>& elements, size_t view_index = 0)
	{
		if (elements.empty())
			view.clear_view();
		else
		{
			view.resize_view(std::max(view_index + elements.size(), view.view_size()));
			for (size_t i = 0; i < elements.size(); ++i)
				update_view(view, elements[i], view_index + i);
		}
	}

	inline CollisionObjectView collision_view(const col2d::Compound& c, glm::vec4 color = STANDARD_BLUE)
	{
		return collision_view(c.elements, color);
	}

	inline void update_view(CollisionView& view, const col2d::Compound& c, size_t view_index = 0)
	{
		update_view(view, c.elements, view_index);
	}

	inline CollisionObjectView collision_view(const col2d::TCompound& c, glm::vec4 color = STANDARD_BLUE)
	{
		return collision_view(c.get_baked(), color);
	}

	inline void update_view(CollisionView& view, const col2d::TCompound& c, size_t view_index = 0)
	{
		update_view(view, c.get_baked(), view_index);
	}

	template<typename Shape>
	inline CollisionObjectView collision_view(const col2d::BVH<Shape>& c, glm::vec4 color = STANDARD_BLUE)
	{
		return collision_view(c.get_elements(), color);
	}

	template<typename Shape>
	inline CollisionObjectView collision_view_at_depth(const col2d::BVH<Shape>& c, size_t depth, glm::vec4 color = STANDARD_BLUE)
	{
		return collision_view(c.build_layer(depth), color);
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::BVH<Shape>& c, size_t view_index = 0)
	{
		update_view(view, c.get_elements(), view_index);
	}

	template<typename Shape>
	inline void update_view_at_depth(CollisionView& view, const col2d::BVH<Shape>& c, size_t depth, size_t view_index = 0)
	{
		update_view(view, c.build_layer(depth), view_index);
	}

	template<typename Shape>
	inline CollisionObjectView collision_view(const col2d::TBVH<Shape>& c, glm::vec4 color = STANDARD_BLUE)
	{
		return collision_view(c.get_baked(), color);
	}

	template<typename Shape>
	inline CollisionObjectView collision_view_at_depth(const col2d::TBVH<Shape>& c, size_t depth, glm::vec4 color = STANDARD_BLUE)
	{
		return collision_view(c.build_layer(depth), color);
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::TBVH<Shape>& c, size_t view_index = 0)
	{
		update_view(view, c.get_baked(), view_index);
	}

	template<typename Shape>
	inline void update_view_at_depth(CollisionView& view, const col2d::TBVH<Shape>& c, size_t depth, size_t view_index = 0)
	{
		update_view(view, c.build_layer(depth), view_index);
	}
}
