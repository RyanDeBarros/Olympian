#pragma once

#include "physics/collision/methods/Collide.h"
#include "physics/collision/objects/Capsule.h"
#include "physics/collision/Tolerance.h"

namespace oly::col2d
{
	struct RectCast
	{
		Ray ray;
		float width, height;

		OBB finite_obb() const
		{
			return OBB{ .center = ray.origin + 0.5f * ray.clip * (glm::vec2)ray.direction, .width = width + ray.clip, .height = height, .rotation = ray.direction.rotation() };
		}
	};

	struct CircleCast
	{
		Ray ray;
		float radius;

		Capsule finite_capsule() const
		{
			return Capsule{ .center = ray.origin + 0.5f * ray.clip * (glm::vec2)ray.direction, .obb_width = 2.0f * radius, .obb_height = ray.clip, .rotation = ray.direction.rotation() };
		}
	};

	template<typename Shape>
	inline OverlapResult rect_cast_hits(const Shape& c, const RectCast& cast)
	{
		if (cast.ray.clip == 0.0f)
		{
			RectCast finite = cast;
			finite.ray.clip = std::max(c.projection_max(cast.ray.direction) - cast.ray.direction.dot(cast.ray.origin), 0.0f);
			return overlaps(c, finite.finite_obb());
		}
		else
			return overlaps(c, cast.finite_obb());
	}

	template<typename Shape>
	inline OverlapResult circle_cast_hits(const Shape& c, const CircleCast& cast)
	{
		if (cast.ray.clip == 0.0f)
		{
			CircleCast finite = cast;
			finite.ray.clip = std::max(c.projection_max(cast.ray.direction) - cast.ray.direction.dot(cast.ray.origin), 0.0f);
			return overlaps(c, finite.finite_capsule().compound());
		}
		else
			return overlaps(c, cast.finite_capsule().compound());
	}
}
