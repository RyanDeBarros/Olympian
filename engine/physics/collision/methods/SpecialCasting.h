#pragma once

#include "physics/collision/methods/Collide.h"
#include "physics/collision/objects/Capsule.h"
#include "physics/collision/Tolerance.h"

namespace oly::col2d
{
	struct RectCast
	{
		Ray ray;
		float width, depth;

		OBB finite_obb(float infinite_clip = 0.0f) const
		{
			float clip = ray.clip == 0.0f ? infinite_clip : ray.clip;
			return OBB{ .center = ray.origin + 0.5f * clip * (glm::vec2)ray.direction, .width = depth + clip, .height = width, .rotation = ray.direction.rotation() };
		}
	};

	struct CircleCast
	{
		Ray ray;
		float radius;

		Capsule finite_capsule(float infinite_clip = 0.0f) const
		{
			float clip = ray.clip == 0.0f ? infinite_clip : ray.clip;
			return Capsule{ .center = ray.origin + 0.5f * clip * (glm::vec2)ray.direction, .obb_width = 2.0f * radius, .obb_height = clip, .rotation = (-ray.direction.get_quarter_turn()).rotation()};
		}
	};

	template<typename Shape>
	inline OverlapResult rect_cast_hits(const Shape& c, const RectCast& cast)
	{
		if (cast.ray.clip == 0.0f)
		{
			RectCast finite = cast;
			finite.ray.clip = std::max(c.projection_max(cast.ray.direction) - cast.ray.direction.dot(cast.ray.origin), 0.0f);
			return overlaps(c, param(finite.finite_obb()));
		}
		else
			return overlaps(c, param(cast.finite_obb()));
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
