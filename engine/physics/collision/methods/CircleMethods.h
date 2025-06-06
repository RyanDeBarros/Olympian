#pragma once

#include "core/base/Transforms.h"

#include "physics/collision/elements/Circle.h"
#include "physics/collision/methods/CollisionInfo.h"

namespace oly::col2d::internal
{
	template<typename Polygon>
	inline OverlapResult circle_overlaps_polygon(const Circle& c, const Polygon& polygon)
	{
		std::optional<bool> ccw;
		glm::vec2 prev = transform_point(col2d::internal::CircleGlobalAccess::get_ginv(c), polygon[polygon.size() - 1]);
		for (size_t i = 0; i < polygon.size(); ++i)
		{
			glm::vec2 a = prev;
			glm::vec2 b = transform_point(col2d::internal::CircleGlobalAccess::get_ginv(c), polygon[i]);
			prev = b;

			UnitVector2D edge = b - a;
			UnitVector2D axis = edge.get_quarter_turn();
			float center_proj = axis.dot(c.center);
			float segment_proj = axis.dot(a);
			static const auto circle_intersects_edge = [](const Circle& c, const UnitVector2D& edge, glm::vec2 a, glm::vec2 b)
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
				if (segment_proj < center_proj - c.radius) // circle is left of edge
					return false;
				else if (circle_intersects_edge(c, edge, a, b))
					return true;
			}
			else
			{
				if (segment_proj > center_proj + c.radius) // circle is right of edge
					return false;
				else if (circle_intersects_edge(c, edge, a, b))
					return true;
			}
		}
		return true;
	}

	template<typename Polygon>
	inline CollisionResult circle_collides_polygon(const Circle& c, const Polygon& polygon)
	{
		CollisionResult info{ .overlap = true, .penetration_depth = nmax<float>() };
		std::optional<bool> ccw;
		glm::vec2 prev = transform_point(col2d::internal::CircleGlobalAccess::get_ginv(c), polygon[polygon.size() - 1]);
		float ccw_depth = nmax<float>();
		UnitVector2D ccw_unit_impulse;
		float cw_depth = nmax<float>();
		UnitVector2D cw_unit_impulse;
		for (size_t i = 0; i < polygon.size(); ++i)
		{
			glm::vec2 a = prev;
			glm::vec2 b = transform_point(col2d::internal::CircleGlobalAccess::get_ginv(c), polygon[i]);
			prev = b;

			UnitVector2D edge = b - a;
			UnitVector2D axis = edge.get_quarter_turn();
			float center_proj = axis.dot(c.center);
			float segment_proj = axis.dot(a);
			if (!ccw.has_value())
			{
				float _ccw_depth = center_proj + c.radius - segment_proj;
				if (_ccw_depth < ccw_depth)
				{
					ccw_depth = _ccw_depth;
					ccw_unit_impulse = -axis;
				}
				float _cw_depth = segment_proj - center_proj + c.radius;
				if (_cw_depth < cw_depth)
				{
					cw_depth = _cw_depth;
					cw_unit_impulse = axis;
				}
				// set winding order
				if (segment_proj < center_proj - c.radius)
				{
					ccw = true;
					info.penetration_depth = ccw_depth;
					info.unit_impulse = ccw_unit_impulse;
				}
				else if (segment_proj > center_proj + c.radius)
				{
					ccw = false;
					info.penetration_depth = cw_depth;
					info.unit_impulse = cw_unit_impulse;
				}
			}
			else if (ccw.value())
			{
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
			else
			{
				if (segment_proj > center_proj + c.radius) // circle is right of edge
					return { .overlap = false };
				else
				{
					float inward_depth = segment_proj - center_proj + c.radius;
					if (inward_depth < info.penetration_depth)
					{
						info.penetration_depth = inward_depth;
						info.unit_impulse = axis;
					}
				}
			}
		}
		if (!ccw.has_value())
		{
			if (ccw_depth < cw_depth)
			{
				info.penetration_depth = ccw_depth;
				info.unit_impulse = ccw_unit_impulse;
			}
			else
			{
				info.penetration_depth = cw_depth;
				info.unit_impulse = cw_unit_impulse;
			}
		}
		if (!internal::CircleGlobalAccess::has_no_global(c))
		{
			// transform back to global
			glm::vec2 impulse = info.penetration_depth * (glm::vec2)info.unit_impulse;
			impulse = transform_direction(col2d::internal::CircleGlobalAccess::get_global(c), impulse);
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
