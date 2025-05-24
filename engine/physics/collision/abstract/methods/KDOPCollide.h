#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"
#include "physics/collision/abstract/methods/SAT.h"

#include "physics/collision/abstract/primitives/KDOP.h"
#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/primitives/AABB.h"
#include "physics/collision/abstract/primitives/OBB.h"
#include "physics/collision/abstract/primitives/ConvexHull.h"
#include "physics/collision/abstract/primitives/Capsule.h"
#include "physics/collision/abstract/primitives/HalfPlane.h"

namespace oly::acm2d
{
	// Matched

	// KDOP
	template<size_t K_half>
	inline OverlapResult point_hits(const KDOP<K_half>& c, glm::vec2 test)
	{
		for (size_t i = 0; i < K_half; ++i)
		{
			float proj = glm::dot(test, KDOP<K_half>::uniform_axis(i));
			if (proj < c.get_minimum(i) || proj > c.get_maximum(i))
				return false;
		}
		return true;
	}
	
	template<size_t K_half>
	inline OverlapResult ray_hits(const KDOP<K_half>& c, const Ray& ray)
	{
		for (size_t i = 0; i < K_half; ++i)
		{
			glm::vec2 axis = KDOP<K_half>::uniform_axis(i);
			float proj_origin = glm::dot(ray.origin, axis);
			float proj_direction = glm::dot((glm::vec2)ray.direction, axis);

			float proj_clip;
			if (ray.clip == 0.0f)
			{
				if (near_zero(proj_direction))
					proj_clip = proj_origin;
				else if (above_zero(proj_direction))
					proj_clip = std::numeric_limits<float>::max();
				else if (below_zero(proj_direction))
					proj_clip = std::numeric_limits<float>::lowest();
			}
			else
				proj_clip = proj_origin + ray.clip * proj_direction;

			float proj_min = std::min(proj_origin, proj_clip);
			float proj_max = std::max(proj_origin, proj_clip);

			if (proj_max < c.get_minimum(i) || proj_min > c.get_maximum(i))
				return false;
		}
		return true;
	}
	
	template<size_t K_half>
	inline RaycastResult raycast(const KDOP<K_half>& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::EMBEDDED_ORIGIN, .contact = ray.origin };
		float t_max = std::numeric_limits<float>::lowest(); // first slab encountered must come at maximum t
		for (size_t i = 0; i < K_half; ++i)
		{
			UnitVector2D axis = KDOP<K_half>::uniform_axis(i);
			float proj_origin = glm::dot(ray.origin, (glm::vec2)axis);
			float proj_direction = glm::dot((glm::vec2)ray.direction, (glm::vec2)axis);

			float proj_clip;
			if (ray.clip == 0.0f)
			{
				if (near_zero(proj_direction))
					proj_clip = proj_origin;
				else if (above_zero(proj_direction))
					proj_clip = std::numeric_limits<float>::max();
				else if (below_zero(proj_direction))
					proj_clip = std::numeric_limits<float>::lowest();
			}
			else
				proj_clip = proj_origin + ray.clip * proj_direction;

			float proj_min = std::min(proj_origin, proj_clip);
			float proj_max = std::max(proj_origin, proj_clip);

			if (proj_max < c.get_minimum(i) || proj_min > c.get_maximum(i))
				return { .hit = RaycastResult::Hit::NO_HIT };

			if (!near_zero(proj_direction)) // ray is not parallel
			{
				// boundary = proj_origin + t * proj_direction --> t = (boundary - proj_origin) / proj_direction
				if (proj_origin < c.get_minimum(i))
				{
					float t = (c.get_minimum(i) - proj_origin) / proj_direction; // boundary == minimum
					if (above_zero(t) && t > t_max)
					{
						t_max = t;
						info.hit = RaycastResult::Hit::TRUE_HIT;
						info.normal = -axis;
					}
				}
				else if (proj_origin > c.get_maximum(i))
				{
					float t = (c.get_maximum(i) - proj_origin) / proj_direction; // boundary == maximum
					if (above_zero(t) && t > t_max)
					{
						t_max = t;
						info.hit = RaycastResult::Hit::TRUE_HIT;
						info.normal = axis;
					}
				}
			}
		}

		if (info.hit == RaycastResult::Hit::TRUE_HIT)
			info.contact = ray.origin + t_max * (glm::vec2)ray.direction;

		return info;
	}
	
	template<size_t K_half1, size_t K_half2>
	inline OverlapResult overlaps(const KDOP<K_half1>& c1, const KDOP<K_half2>& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::overlaps(c1, c2);
	}
	
	template<size_t K_half1, size_t K_half2>
	inline CollisionResult collides(const KDOP<K_half1>& c1, const KDOP<K_half2>& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::collides(c1, c2);
	}
	
	template<size_t K_half1, size_t K_half2>
	inline ContactResult contacts(const KDOP<K_half1>& c1, const KDOP<K_half2>& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::contacts(c1, c2);
	}

	// Mixed

}
