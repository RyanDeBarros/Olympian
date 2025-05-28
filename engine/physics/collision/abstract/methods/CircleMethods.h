#pragma once

#include "core/base/Transforms.h"

#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/methods/CollisionInfo.h"

namespace oly::acm2d::internal
{
	template<typename Polygon>
	inline OverlapResult circle_overlaps_polygon(const Circle& c, const Polygon& polygon)
	{
		for (size_t i = 0; i < polygon.size(); ++i)
		{
			glm::vec2 a = polygon[i];
			a = transform_point(acm2d::internal::CircleGlobalAccess::get_ginv(c), a);
			glm::vec2 b = polygon[(i + 1) % polygon.size()];
			b = transform_point(acm2d::internal::CircleGlobalAccess::get_ginv(c), b);

			UnitVector2D edge = b - a;
			UnitVector2D axis = edge.get_quarter_turn();
			float center_proj = axis.dot(c.center);
			float segment_proj = axis.dot(a);
			if (segment_proj < center_proj - c.radius) // circle is left of edge
				return false;
			else if (segment_proj <= center_proj + c.radius) // circle may intersect edge
			{
				float cross = math::cross(edge, c.center - a);
				float discriminant = c.radius * c.radius - cross * cross;
				if (discriminant >= 0.0f)
				{
					float offset = edge.dot(c.center - a);
					discriminant = glm::sqrt(discriminant);
					float t1 = offset - discriminant;
					float t2 = offset + discriminant;

					if (t2 >= 0.0f && t1 <= glm::length(b - a)) // circle intersects edge
						return true;
				}
			}
		}
		return true;
	}

	template<typename Polygon>
	inline CollisionResult circle_collides_polygon(const Circle& c, const Polygon& polygon)
	{
		CollisionResult info{ .overlap = true, .penetration_depth = std::numeric_limits<float>::max() };
		for (size_t i = 0; i < polygon.size(); ++i)
		{
			glm::vec2 a = polygon[i];
			a = transform_point(acm2d::internal::CircleGlobalAccess::get_ginv(c), a);
			glm::vec2 b = polygon[(i + 1) % polygon.size()];
			b = transform_point(acm2d::internal::CircleGlobalAccess::get_ginv(c), b);

			UnitVector2D edge = b - a;
			UnitVector2D axis = edge.get_quarter_turn();
			float center_proj = axis.dot(c.center);
			float segment_proj = axis.dot(a);
			if (segment_proj < center_proj - c.radius) // circle is left of edge
				return { .overlap = false };
			else
			{
				float inward_depth = center_proj + c.radius - segment_proj;
				if (inward_depth < info.penetration_depth)
				{
					info.penetration_depth = inward_depth;
					info.unit_impulse = -axis;
				}
			}
		}
		if (!internal::CircleGlobalAccess::has_no_global(c))
		{
			// transform back to global
			glm::vec2 impulse = info.penetration_depth * (glm::vec2)info.unit_impulse;
			impulse = transform_direction(acm2d::internal::CircleGlobalAccess::get_global(c), impulse);
			info.penetration_depth = glm::length(impulse);
			info.unit_impulse = impulse;
		}
		return info;
	}

	template<typename Shape, typename Polygon>
	inline ContactResult circle_contacts_polygon(const Circle& c, const Shape& other, const Polygon& polygon)
	{
		CollisionResult collision = circle_collides_polygon(c, polygon);
		if (!collision.overlap)
			return { .overlap = false };

		return {
			.overlap = true,
			.active_feature = {
				.position = c.deepest_point(-collision.unit_impulse),
				.impulse = (glm::vec2)collision.unit_impulse * collision.penetration_depth
			},
			.static_feature = {
				.position = other.deepest_point(collision.unit_impulse),
				.impulse = -(glm::vec2)collision.unit_impulse * collision.penetration_depth,
			}
		};
	}
}
