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
			if (!internal::ray_hits_slab(c.get_minimum(i), c.get_maximum(i), ray, KDOP<K_half>::uniform_axis(i)))
				return false;
		}
		return true;
	}
	
	template<size_t K_half>
	inline RaycastResult raycast(const KDOP<K_half>& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::EMBEDDED_ORIGIN, .contact = ray.origin };
		float max_entry = std::numeric_limits<float>::lowest();
		for (size_t i = 0; i < K_half; ++i)
		{
			if (!internal::raycast_update_on_slab(c.get_minimum(i), c.get_maximum(i), ray, KDOP<K_half>::uniform_axis(i), info, max_entry))
				return { .hit = RaycastResult::Hit::NO_HIT };
		}
		if (info.hit == RaycastResult::Hit::TRUE_HIT)
			info.contact = ray.origin + max_entry * (glm::vec2)ray.direction;
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
