#pragma once

#include "physics/collision/debugging/CollisionView.h"

namespace oly::debug
{
	constexpr glm::vec4 STANDARD_BLUE = glm::vec4{ 0.0f, 0.0f, 1.0f, 0.8f };

	inline CollisionView collision_view(const col2d::Circle& c, glm::vec4 color)
	{
		rendering::EllipseBatch::EllipseReference ellipse;
		ellipse.set_transform() = augment(col2d::internal::CircleGlobalAccess::get_global(c), col2d::internal::CircleGlobalAccess::get_global_offset(c)) * translation_matrix(c.center);
		auto& dim = ellipse.set_dimension();
		dim.ry = dim.rx = c.radius;
		ellipse.set_color().fill_outer = color;
		return CollisionView(std::move(ellipse));
	}

	inline void update_view(CollisionView& view, const col2d::Circle& c, glm::vec4 color, size_t view_index = 0)
	{
		CollisionObject& obj = view.get_view(view_index);
		bool modify = std::visit([](auto&& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), rendering::EllipseBatch::EllipseReference>)
				return true;
			else
				return false;
			}, obj);

		if (modify)
		{
			rendering::EllipseBatch::EllipseReference& ellipse = std::get<CollisionObjectType::ELLIPSE>(obj);
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

	inline void update_view_no_color(CollisionView& view, const col2d::Circle& c, size_t view_index = 0)
	{
		CollisionObject& obj = view.get_view(view_index);
		bool modify = std::visit([](auto&& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), rendering::EllipseBatch::EllipseReference>)
				return true;
			else
				return false;
			}, obj);

		if (modify)
		{
			rendering::EllipseBatch::EllipseReference& ellipse = std::get<CollisionObjectType::ELLIPSE>(obj);
			ellipse.set_transform() = augment(col2d::internal::CircleGlobalAccess::get_global(c), col2d::internal::CircleGlobalAccess::get_global_offset(c)) * translation_matrix(c.center);
			auto& dim = ellipse.set_dimension();
			dim.ry = dim.rx = c.radius;
			view.view_changed();
		}
		else
		{
			rendering::EllipseBatch::EllipseReference ellipse;
			ellipse.set_transform() = augment(col2d::internal::CircleGlobalAccess::get_global(c), col2d::internal::CircleGlobalAccess::get_global_offset(c)) * translation_matrix(c.center);
			auto& dim = ellipse.set_dimension();
			dim.ry = dim.rx = c.radius;
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
		inline void polygon_update_view(debug::CollisionView& view, const Polygon& points, glm::vec4 color, size_t view_index = 0)
		{
			if (points.size() < 3)
			{
				view.clear_view();
				return;
			}

			CollisionObject& obj = view.get_view(view_index);
			bool modify = std::visit([](auto&& obj) -> bool {
				if constexpr (visiting_class_is<decltype(obj), rendering::StaticPolygon>)
					return true;
				else
					return false;
				}, obj);

			if (modify)
			{
				rendering::StaticPolygon& polygon = std::get<CollisionObjectType::POLYGON>(obj);
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

		template<typename Polygon>
		inline void polygon_update_view_no_color(debug::CollisionView& view, const Polygon& points, size_t view_index = 0)
		{
			if (points.size() < 3)
			{
				view.clear_view();
				return;
			}

			CollisionObject& obj = view.get_view(view_index);
			bool modify = std::visit([](auto&& obj) -> bool {
				if constexpr (visiting_class_is<decltype(obj), rendering::StaticPolygon>)
					return true;
				else
					return false;
				}, obj);

			if (modify)
			{
				rendering::StaticPolygon& polygon = std::get<CollisionObjectType::POLYGON>(obj);
				polygon.polygon.points.clear();
				polygon.polygon.points.insert(polygon.polygon.points.end(), points.begin(), points.end());
				polygon.send_polygon();
				view.view_changed();
			}
			else
			{
				rendering::StaticPolygon polygon;
				polygon.polygon.colors = { glm::vec4{ 0.0f, 0.0f, 1.0f, 0.8f } };
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

	inline void update_view(CollisionView& view, const col2d::AABB& c, glm::vec4 color, size_t view_index = 0)
	{
		internal::polygon_update_view(view, c.points(), color, view_index);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::AABB& c, size_t view_index = 0)
	{
		internal::polygon_update_view_no_color(view, c.points(), view_index);
	}

	inline CollisionView collision_view(const col2d::OBB& c, glm::vec4 color)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::OBB& c, glm::vec4 color, size_t view_index = 0)
	{
		internal::polygon_update_view(view, c.points(), color, view_index);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::OBB& c, glm::vec4 color, size_t view_index = 0)
	{
		internal::polygon_update_view_no_color(view, c.points(), view_index);
	}

	inline CollisionView collision_view(const col2d::ConvexHull& c, glm::vec4 color)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	inline void update_view(CollisionView& view, const col2d::ConvexHull& c, glm::vec4 color, size_t view_index = 0)
	{
		internal::polygon_update_view(view, c.points(), color, view_index);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::ConvexHull& c, size_t view_index = 0)
	{
		internal::polygon_update_view_no_color(view, c.points(), view_index);
	}

	template<size_t K>
	inline CollisionView collision_view(const col2d::KDOP<K>& c, glm::vec4 color)
	{
		return internal::polygon_collision_view(c.points(), color);
	}

	template<size_t K>
	inline void update_view(CollisionView& view, const col2d::KDOP<K>& c, glm::vec4 color, size_t view_index = 0)
	{
		internal::polygon_update_view(view, c.points(), color, view_index);
	}

	template<size_t K>
	inline void update_view_no_color(CollisionView& view, const col2d::KDOP<K>& c, size_t view_index = 0)
	{
		internal::polygon_update_view_no_color(view, c.points(), view_index);
	}

	inline CollisionView collision_view(const col2d::Element& c, glm::vec4 color)
	{
		return std::visit([color](auto&& e) { return collision_view(*e, color); }, param(c));
	}

	inline void update_view(CollisionView& view, const col2d::Element& c, glm::vec4 color, size_t view_index = 0)
	{
		std::visit([&view, color, view_index](auto&& e) { update_view(view, *e, color, view_index); }, param(c));
	}

	inline void update_view_no_color(CollisionView& view, const col2d::Element& c, size_t view_index = 0)
	{
		std::visit([&view, view_index](auto&& e) { update_view_no_color(view, *e, view_index); }, param(c));
	}

	inline CollisionView collision_view(const col2d::ElementParam& c, glm::vec4 color)
	{
		return std::visit([color](auto&& e) { return collision_view(*e, color); }, c);
	}

	inline void update_view(CollisionView& view, const col2d::ElementParam& c, glm::vec4 color, size_t view_index = 0)
	{
		std::visit([&view, color, view_index](auto&& e) { update_view(view, *e, color, view_index); }, c);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::ElementParam& c, size_t view_index = 0)
	{
		std::visit([&view, view_index](auto&& e) { update_view_no_color(view, *e, view_index); }, c);
	}

	inline CollisionView collision_view(const col2d::Primitive& c, glm::vec4 color)
	{
		return collision_view(c.element, color);
	}

	inline void update_view(CollisionView& view, const col2d::Primitive& c, glm::vec4 color, size_t view_index = 0)
	{
		update_view(view, c.element, color, view_index);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::Primitive& c, size_t view_index = 0)
	{
		update_view_no_color(view, c.element, view_index);
	}

	inline CollisionView collision_view(const col2d::TPrimitive& c, glm::vec4 color)
	{
		return collision_view(c.get_baked(), color);
	}

	inline void update_view(CollisionView& view, const col2d::TPrimitive& c, glm::vec4 color, size_t view_index = 0)
	{
		update_view(view, c.get_baked(), color, view_index);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::TPrimitive& c, size_t view_index = 0)
	{
		update_view_no_color(view, c.get_baked(), view_index);
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
	inline void update_view(CollisionView& view, const std::vector<Object>& elements, glm::vec4 color, size_t view_index = 0)
	{
		if (elements.empty())
			view.clear_view();
		else
		{
			view.resize_view(std::max(view_index + elements.size(), view.view_size()));
			for (size_t i = 0; i < elements.size(); ++i)
				update_view(view, elements[i], color, view_index + i);
		}
	}

	template<typename Object>
	inline void update_view_no_color(CollisionView& view, const std::vector<Object>& elements, size_t view_index = 0)
	{
		if (elements.empty())
			view.clear_view();
		else
		{
			view.resize_view(std::max(view_index + elements.size(), view.view_size()));
			for (size_t i = 0; i < elements.size(); ++i)
				update_view_no_color(view, elements[i], view_index + i);
		}
	}

	inline CollisionView collision_view(const col2d::Compound& c, glm::vec4 color)
	{
		return collision_view(c.elements, color);
	}

	inline void update_view(CollisionView& view, const col2d::Compound& c, glm::vec4 color, size_t view_index = 0)
	{
		update_view(view, c.elements, color, view_index);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::Compound& c, size_t view_index = 0)
	{
		update_view_no_color(view, c.elements, view_index);
	}

	inline CollisionView collision_view(const col2d::TCompound& c, glm::vec4 color)
	{
		return collision_view(c.get_baked(), color);
	}

	inline void update_view(CollisionView& view, const col2d::TCompound& c, glm::vec4 color, size_t view_index = 0)
	{
		update_view(view, c.get_baked(), color, view_index);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::TCompound& c, size_t view_index = 0)
	{
		update_view_no_color(view, c.get_baked(), view_index);
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::BVH<Shape>& c, glm::vec4 color)
	{
		return collision_view(c.get_elements(), color);
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::BVH<Shape>& c, size_t depth, glm::vec4 color)
	{
		return collision_view(c.build_layer(depth), color);
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::BVH<Shape>& c, glm::vec4 color, size_t view_index = 0)
	{
		update_view(view, c.get_elements(), color, view_index);
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::BVH<Shape>& c, size_t depth, glm::vec4 color, size_t view_index = 0)
	{
		update_view(view, c.build_layer(depth), color, view_index);
	}

	template<typename Shape>
	inline void update_view_no_color(CollisionView& view, const col2d::BVH<Shape>& c, size_t view_index = 0)
	{
		update_view_no_color(view, c.get_elements(), view_index);
	}

	template<typename Shape>
	inline void update_view_no_color(CollisionView& view, const col2d::BVH<Shape>& c, size_t depth, size_t view_index = 0)
	{
		update_view_no_color(view, c.build_layer(depth), view_index);
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::TBVH<Shape>& c, glm::vec4 color)
	{
		return collision_view(c.get_baked(), color);
	}

	template<typename Shape>
	inline CollisionView collision_view(const col2d::TBVH<Shape>& c, size_t depth, glm::vec4 color)
	{
		return collision_view(c.build_layer(depth), color);
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::TBVH<Shape>& c, glm::vec4 color, size_t view_index = 0)
	{
		update_view(view, c.get_baked(), color, view_index);
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::TBVH<Shape>& c, size_t depth, glm::vec4 color, size_t view_index = 0)
	{
		update_view(view, c.build_layer(depth), color, view_index);
	}

	template<typename Shape>
	inline void update_view_no_color(CollisionView& view, const col2d::TBVH<Shape>& c, size_t view_index = 0)
	{
		update_view_no_color(view, c.get_baked(), view_index);
	}

	template<typename Shape>
	inline void update_view(CollisionView& view, const col2d::TBVH<Shape>& c, size_t depth, size_t view_index = 0)
	{
		update_view_no_color(view, c.build_layer(depth), view_index);
	}
}
