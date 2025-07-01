#pragma once

#include "core/base/Transforms.h"

#include "physics/collision/elements/Circle.h"
#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/Tolerance.h"

namespace oly::col2d::internal
{
	static bool circle_intersects_edge(const Circle& c, const UnitVector2D& edge, glm::vec2 a, glm::vec2 b)
	{
		float cross = math::cross(edge, c.center - a);
		float discriminant = c.radius * c.radius - cross * cross;
		if (discriminant >= 0.0f)
		{
			float offset = edge.dot(c.center - a);
			discriminant = glm::sqrt(discriminant);
			float t1 = offset - discriminant;
			float t2 = offset + discriminant;

			if (t2 >= 0.0f && t1 <= glm::length(b - a))
				return true;
		}
		return false;
	};

	template<typename Polygon>
	inline OverlapResult circle_overlaps_polygon(const Circle& c, const Polygon& polygon)
	{
		std::optional<bool> ccw;
		glm::vec2 prev = col2d::internal::CircleGlobalAccess::local_point(c, polygon[polygon.size() - 1]);
		glm::vec2 local_polygon_center = prev;
		for (size_t i = 0; i < polygon.size(); ++i)
		{
			glm::vec2 a = prev;
			glm::vec2 b = col2d::internal::CircleGlobalAccess::local_point(c, polygon[i]);
			prev = b;
			local_polygon_center += prev;

			UnitVector2D edge = b - a;
			UnitVector2D axis = edge.get_quarter_turn();
			float center_proj = axis.dot(c.center);
			float segment_proj = axis.dot(a);
			if (!ccw.has_value())
			{
				if (segment_proj < center_proj - c.radius)
					ccw = true;
				else if (segment_proj > center_proj + c.radius)
					ccw = false;
				else if (circle_intersects_edge(c, edge, a, b))
					return true;
			}
			else if (ccw.value())
			{
				if (segment_proj > center_proj + c.radius) // circle is outwards from edge
					return false;
				else if (circle_intersects_edge(c, edge, a, b))
					return true;
			}
			else
			{
				if (segment_proj < center_proj - c.radius) // circle is outwards from edge
					return false;
				else if (circle_intersects_edge(c, edge, a, b))
					return true;
			}
		}
		return math::mag_sqrd(local_polygon_center / (float)polygon.size() - c.center) <= c.radius * c.radius; // circle encloses polygon
	}

	template<typename Polygon>
	inline CollisionResult circle_collides_polygon(const Circle& c, const Polygon& polygon)
	{
		CollisionResult info{ .overlap = false, .penetration_depth = nmax<float>() };
		std::optional<bool> ccw;
		glm::vec2 prev = col2d::internal::CircleGlobalAccess::local_point(c, polygon[polygon.size() - 1]);
		glm::vec2 local_polygon_center = prev;
		float ccw_depth = nmax<float>();
		UnitVector2D ccw_unit_impulse;
		float cw_depth = nmax<float>();
		UnitVector2D cw_unit_impulse;
		for (size_t i = 0; i < polygon.size(); ++i)
		{
			glm::vec2 a = prev;
			glm::vec2 b = col2d::internal::CircleGlobalAccess::local_point(c, polygon[i]);
			prev = b;
			local_polygon_center += prev;

			UnitVector2D edge = b - a;
			UnitVector2D axis = edge.get_quarter_turn();
			float center_proj = axis.dot(c.center);
			float segment_proj = axis.dot(a);

			if (circle_intersects_edge(c, edge, a, b))
				info.overlap = true;

			if (!ccw.has_value())
			{
				if (segment_proj < center_proj - c.radius)
				{
					ccw = true;
					float depth = center_proj + c.radius - segment_proj;
					if (depth < ccw_depth)
					{
						ccw_depth = depth;
						ccw_unit_impulse = -axis;
					}
					info.penetration_depth = ccw_depth;
					info.unit_impulse = ccw_unit_impulse;
				}
				else if (segment_proj > center_proj + c.radius)
				{
					ccw = false;
					float depth = segment_proj - (center_proj - c.radius);
					if (depth < cw_depth)
					{
						ccw_depth = depth;
						ccw_unit_impulse = axis;
					}
					info.penetration_depth = cw_depth;
					info.unit_impulse = cw_unit_impulse;
				}
				else
				{
					float depth = center_proj + c.radius - segment_proj;
					if (depth < ccw_depth)
					{
						ccw_depth = depth;
						ccw_unit_impulse = -axis;
					}
					depth = segment_proj - (center_proj - c.radius);
					if (depth < cw_depth)
					{
						cw_depth = depth;
						cw_unit_impulse = axis;
					}
				}
			}
			else if (ccw.value())
			{
				if (segment_proj > center_proj + c.radius) // circle is outwards from edge
					return { .overlap = false };
				else
				{
					float depth = center_proj + c.radius - segment_proj;
					if (depth < info.penetration_depth)
					{
						info.penetration_depth = depth;
						info.unit_impulse = -axis;
					}
				}
			}
			else
			{
				if (segment_proj < center_proj - c.radius) // circle is outwards from edge
					return { .overlap = false };
				else
				{
					float depth = segment_proj - (center_proj - c.radius);
					if (depth < info.penetration_depth)
					{
						info.penetration_depth = depth;
						info.unit_impulse = axis;
					}
				}
			}
		}
		if (!info.overlap && math::mag_sqrd(local_polygon_center / (float)polygon.size() - c.center) > c.radius * c.radius) // circle does not enclose polygon
			return { .overlap = false };

		info.overlap = true;
		if (ccw_depth < std::min(cw_depth, info.penetration_depth))
		{
			info.penetration_depth = ccw_depth;
			info.unit_impulse = ccw_unit_impulse;
		}
		else if (cw_depth < std::min(ccw_depth, info.penetration_depth))
		{
			info.penetration_depth = cw_depth;
			info.unit_impulse = cw_unit_impulse;
		}

		if (!internal::CircleGlobalAccess::has_no_global(c))
		{
			// transform back to global
			glm::vec2 impulse = info.penetration_depth * (glm::vec2)info.unit_impulse;
			impulse = col2d::internal::CircleGlobalAccess::global_direction(c, impulse);
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
			.passive_feature = {
				.position = other.deepest_point(collision.unit_impulse),
				.impulse = -(glm::vec2)collision.unit_impulse * collision.penetration_depth,
			}
		};
	}
}
