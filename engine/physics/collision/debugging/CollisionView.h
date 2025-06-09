#pragma once

#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"

#include "graphics/primitives/Ellipses.h"
#include "graphics/primitives/Polygons.h"
#include "graphics/extensions/Arrow.h"

namespace oly::debug
{
	// TODO cache references to EllipseReference/StaticPolygon so that list doesn't need to be rebuilt every frame.
	namespace internal
	{
		class CollisionViewImpl
		{
			mutable std::vector<std::variant<rendering::EllipseBatch::EllipseReference, rendering::StaticPolygon, rendering::StaticArrowExtension>> objects;

		public:
			void append(rendering::EllipseBatch::EllipseReference&& obj) const { objects.push_back(std::move(obj)); }
			void append(rendering::StaticPolygon&& obj) const { objects.push_back(std::move(obj)); }
			void append(rendering::StaticArrowExtension&& obj) const { objects.push_back(std::move(obj)); }
			void render() const;
		};
	}

	inline internal::CollisionViewImpl CollisionView; // TODO move to context

	inline void draw_collision(const col2d::Circle& c, glm::vec4 color)
	{
		rendering::EllipseBatch::EllipseReference ellipse;
		ellipse.set_transform() = glm::mat3(col2d::internal::CircleGlobalAccess::get_global(c)) * translation_matrix(c.center);
		auto& dim = ellipse.set_dimension();
		dim.ry = dim.rx = c.radius;
		ellipse.set_color().fill_outer = color;
		CollisionView.append(std::move(ellipse));
	}

	namespace internal
	{
		template<typename Polygon>
		static void draw_polygon_collision(const Polygon& points, glm::vec4 color)
		{
			rendering::StaticPolygon polygon;
			polygon.polygon.colors = { color };
			polygon.polygon.points.insert(polygon.polygon.points.end(), points.begin(), points.end());
			polygon.init();
			CollisionView.append(std::move(polygon));
		}
	}

	inline void draw_collision(const col2d::AABB& c, glm::vec4 color)
	{
		internal::draw_polygon_collision(c.points(), color);
	}

	inline void draw_collision(const col2d::OBB& c, glm::vec4 color)
	{
		internal::draw_polygon_collision(c.points(), color);
	}

	inline void draw_collision(const col2d::ConvexHull& c, glm::vec4 color)
	{
		internal::draw_polygon_collision(c.points(), color);
	}

	inline void draw_collision(const col2d::CustomKDOP& c, glm::vec4 color)
	{
		internal::draw_polygon_collision(c.points(), color);
	}

	template<size_t K_half, std::array<UnitVector2D, K_half> Axes>
	inline void draw_collision(const col2d::CustomKDOPShape<K_half, Axes>& c, glm::vec4 color)
	{
		internal::draw_polygon_collision(c.points(), color);
	}

	template<size_t K_half>
	inline void draw_collision(const col2d::KDOP<K_half>& c, glm::vec4 color)
	{
		internal::draw_polygon_collision(c.points(), color);
	}

	inline void draw_collision(const col2d::Element& c, glm::vec4 color)
	{
		std::visit([color](auto&& e) { draw_collision(e, color); }, c);
	}
	
	inline void draw_collision(const col2d::Primitive& c, glm::vec4 color)
	{
		draw_collision(c.element, color);
	}

	inline void draw_collision(const col2d::TPrimitive& c, glm::vec4 color)
	{
		draw_collision(c.get_baked(), color);
	}

	inline void draw_collision(const col2d::Compound& c, glm::vec4 color)
	{
		for (const auto& e : c.elements)
			draw_collision(e, color);
	}

	inline void draw_collision(const col2d::TCompound& c, glm::vec4 color)
	{
		for (const auto& e : c.get_baked())
			draw_collision(e, color);
	}

	template<typename Shape>
	inline void draw_collision(const col2d::BVH<Shape>& c, glm::vec4 color)
	{
		for (const auto& e : c.get_elements())
			draw_collision(e, color);
	}

	template<typename Shape>
	inline void draw_collision(const col2d::TBVH<Shape>& c, glm::vec4 color)
	{
		for (const auto& e : c.get_elements())
			draw_collision(e, color);
	}

	inline void draw_impulse(const col2d::ContactResult::Feature& feature, glm::vec4 color)
	{
		oly::rendering::StaticArrowExtension impulse; // TODO StaticArrowExtension that doesn't have transformer
		impulse.set_color(color);
		impulse.adjust_standard_head_for_width(6.0f);
		impulse.set_start() = feature.position;
		impulse.set_end() = feature.position + feature.impulse;
		CollisionView.append(std::move(impulse));
	}
}
