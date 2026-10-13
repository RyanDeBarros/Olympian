#pragma once

#include "physics/collision/debugging/DebugOverlay.h"

namespace oly::debug
{
	constexpr glm::vec4 kStandardBlue = { 0.0f, 0.0f, 1.0f, 0.8f };
	constexpr glm::vec4 kStandardGreen = { 0.0f, 1.0f, 0.0f, 0.8f };

    extern DebugShapeGroup create_shape_group(const col2d::Circle& c, glm::vec4 color = kStandardBlue);
    extern void modify_shape_group(DebugOverlay& overlay, const col2d::Circle& c, size_t shape_index = 0);

	namespace internal
	{
		template<typename Polygon>
		inline DebugShapeGroup create_polygon_shape_group(const Polygon& points, glm::vec4 color = kStandardBlue)
		{
			if (points.size() < 3)
				return {};

			rendering::StaticPolygon polygon;
			polygon.set_colors() = { color };
			polygon.set_points().insert(polygon.get_points().end(), points.begin(), points.end());
			return polygon;
		}

		template<typename Polygon>
		inline void modify_polygon_shape_group(DebugOverlay& overlay, const Polygon& points, size_t shape_index = 0)
		{
			if (points.size() < 3)
			{
				overlay.clear_shape_group();
				return;
			}

			if (auto obj = overlay[shape_index]->safe_get<rendering::StaticPolygon>())
			{
				obj->set_points().clear();
				obj->set_points().insert(obj->get_points().end(), points.begin(), points.end());
				overlay.shapes_modified();
			}
			else
			{
				rendering::StaticPolygon polygon = overlay.get_layer().create_polygon();
				polygon.set_colors() = { glm::vec4{ 0.0f, 0.0f, 1.0f, 0.8f } };
				polygon.set_points().insert(polygon.get_points().end(), points.begin(), points.end());
				overlay.set_shape_group(std::move(polygon));
			}
		}
	}

    extern DebugShapeGroup create_shape_group(const col2d::AABB& c, glm::vec4 color = kStandardBlue);
    extern void modify_shape_group(DebugOverlay& overlay, const col2d::AABB& c, size_t shape_index = 0);
    
    extern DebugShapeGroup create_shape_group(const col2d::OBB& c, glm::vec4 color = kStandardBlue);
    extern void modify_shape_group(DebugOverlay& overlay, const col2d::OBB& c, size_t shape_index = 0);
    
    extern DebugShapeGroup create_shape_group(const col2d::ConvexHull& c, glm::vec4 color = kStandardBlue);
    extern void modify_shape_group(DebugOverlay& overlay, const col2d::ConvexHull& c, size_t shape_index = 0);

	template<size_t K>
	inline DebugShapeGroup create_shape_group(const col2d::KDOP<K>& c, glm::vec4 color = kStandardBlue)
	{
		return internal::create_polygon_shape_group(c.points(), color);
	}

	template<size_t K>
	inline void modify_shape_group(DebugOverlay& overlay, const col2d::KDOP<K>& c, size_t shape_index = 0)
	{
		internal::modify_polygon_shape_group(overlay, c.points(), shape_index);
	}

    extern DebugShapeGroup create_shape_group(const col2d::Element& c, glm::vec4 color = kStandardBlue);
    extern void modify_shape_group(DebugOverlay& overlay, const col2d::Element& c, size_t shape_index = 0);
    
    extern DebugShapeGroup create_shape_group(const col2d::Primitive& c, glm::vec4 color = kStandardBlue);
    extern void modify_shape_group(DebugOverlay& overlay, const col2d::Primitive& c, size_t shape_index = 0);
    
    extern DebugShapeGroup create_shape_group(const col2d::TPrimitive& c, glm::vec4 color = kStandardBlue);
    extern void modify_shape_group(DebugOverlay& overlay, const col2d::TPrimitive& c, size_t shape_index = 0);

	template<typename Object>
	inline DebugShapeGroup create_shape_group(const std::vector<Object>& elements, glm::vec4 color = kStandardBlue)
	{
		if (elements.empty())
			return {};

		DebugShapeGroup overlay = create_shape_group(elements[0], color);
		for (size_t i = 1; i < elements.size(); ++i)
			overlay.merge(create_shape_group(elements[i], color));
		return overlay;
	}

	template<typename Object>
	inline void modify_shape_group(DebugOverlay& overlay, const std::vector<Object>& elements, size_t shape_index = 0)
	{
		if (elements.empty())
			overlay.clear_shape_group();
		else
		{
			overlay.resize_shape_group(std::max(shape_index + elements.size(), overlay.shape_count()));
			for (size_t i = 0; i < elements.size(); ++i)
				modify_shape_group(overlay, elements[i], shape_index + i);
		}
	}

    extern DebugShapeGroup create_shape_group(const col2d::Compound& c, glm::vec4 color = kStandardBlue);
    extern void modify_shape_group(DebugOverlay& overlay, const col2d::Compound& c, size_t shape_index = 0);
    
    extern DebugShapeGroup create_shape_group(const col2d::TCompound& c, glm::vec4 color = kStandardBlue);
    extern void modify_shape_group(DebugOverlay& overlay, const col2d::TCompound& c, size_t shape_index = 0);

	template<typename Shape>
	inline DebugShapeGroup create_shape_group(const col2d::BVH<Shape>& c, glm::vec4 color = kStandardBlue)
	{
		return create_shape_group(c.get_elements(), color);
	}

	template<typename Shape>
	inline DebugShapeGroup collision_overlay_at_depth(const col2d::BVH<Shape>& c, size_t depth, glm::vec4 color = kStandardBlue)
	{
		return create_shape_group(c.build_layer(depth), color);
	}

	template<typename Shape>
	inline void modify_shape_group(DebugOverlay& overlay, const col2d::BVH<Shape>& c, size_t shape_index = 0)
	{
		modify_shape_group(overlay, c.get_elements(), shape_index);
	}

	template<typename Shape>
	inline void modify_shape_group_at_depth(DebugOverlay& overlay, const col2d::BVH<Shape>& c, size_t depth, size_t shape_index = 0)
	{
		modify_shape_group(overlay, c.build_layer(depth), shape_index);
	}

	template<typename Shape>
	inline DebugShapeGroup create_shape_group(const col2d::TBVH<Shape>& c, glm::vec4 color = kStandardBlue)
	{
		return create_shape_group(c.get_baked(), color);
	}

	template<typename Shape>
	inline DebugShapeGroup collision_overlay_at_depth(const col2d::TBVH<Shape>& c, size_t depth, glm::vec4 color = kStandardBlue)
	{
		return create_shape_group(c.build_layer(depth), color);
	}

	template<typename Shape>
	inline void modify_shape_group(DebugOverlay& overlay, const col2d::TBVH<Shape>& c, size_t shape_index = 0)
	{
		modify_shape_group(overlay, c.get_baked(), shape_index);
	}

	template<typename Shape>
	inline void modify_shape_group_at_depth(DebugOverlay& overlay, const col2d::TBVH<Shape>& c, size_t depth, size_t shape_index = 0)
	{
		modify_shape_group(overlay, c.build_layer(depth), shape_index);
	}
}
