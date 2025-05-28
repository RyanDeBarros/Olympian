#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"
#include "physics/collision/abstract/methods/SAT.h"
#include "physics/collision/abstract/methods/CircleMethods.h"

#include "physics/collision/abstract/primitives/KDOP.h"
#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/primitives/AABB.h"
#include "physics/collision/abstract/primitives/OBB.h"
#include "physics/collision/abstract/primitives/ConvexHull.h"

namespace oly::acm2d
{
	// Matched

	// ######################################################################################################################################################
	// KDOP

	template<size_t K_half>
	inline OverlapResult point_hits(const KDOP<K_half>& c, glm::vec2 test)
	{
		for (size_t i = 0; i < K_half; ++i)
		{
			float proj = KDOP<K_half>::uniform_axis(i).dot(test);
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

	// ######################################################################################################################################################

	// ######################################################################################################################################################
	// CustomKDOP

	inline OverlapResult point_hits(const CustomKDOP& c, glm::vec2 test)
	{
		for (size_t i = 0; i < c.get_k_half(); ++i)
		{
			float proj = c.edge_normal(i).dot(test);
			if (proj < c.get_minimum(i) || proj > c.get_maximum(i))
				return false;
		}
		return true;
	}

	inline OverlapResult ray_hits(const CustomKDOP& c, const Ray& ray)
	{
		for (size_t i = 0; i < c.get_k_half(); ++i)
		{
			if (!internal::ray_hits_slab(c.get_minimum(i), c.get_maximum(i), ray, c.edge_normal(i)))
				return false;
		}
		return true;
	}

	inline RaycastResult raycast(const CustomKDOP& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::EMBEDDED_ORIGIN, .contact = ray.origin };
		float max_entry = std::numeric_limits<float>::lowest();
		for (size_t i = 0; i < c.get_k_half(); ++i)
		{
			if (!internal::raycast_update_on_slab(c.get_minimum(i), c.get_maximum(i), ray, c.edge_normal(i), info, max_entry))
				return { .hit = RaycastResult::Hit::NO_HIT };
		}
		if (info.hit == RaycastResult::Hit::TRUE_HIT)
			info.contact = ray.origin + max_entry * (glm::vec2)ray.direction;
		return info;
	}

	inline OverlapResult overlaps(const CustomKDOP& c1, const CustomKDOP& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::overlaps(c1, c2);
	}

	inline CollisionResult collides(const CustomKDOP& c1, const CustomKDOP& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::collides(c1, c2);
	}

	inline ContactResult contacts(const CustomKDOP& c1, const CustomKDOP& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::contacts(c1, c2);
	}

	// ######################################################################################################################################################

	// Mixed

	// ######################################################################################################################################################
	// KDOP - Circle

	template<size_t K_half>
	inline OverlapResult overlaps(const KDOP<K_half>& c1, const Circle& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_overlaps_polygon(c2, c1.points());
	}
	template<size_t K_half>
	inline OverlapResult overlaps(const Circle& c1, const KDOP<K_half>& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_overlaps_polygon(c1, c2.points());
	}
	template<size_t K_half>
	inline CollisionResult collides(const KDOP<K_half>& c1, const Circle& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_collides_polygon(c2, c1.points()).invert();
	}
	template<size_t K_half>
	inline CollisionResult collides(const Circle& c1, const KDOP<K_half>& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_collides_polygon(c1, c2.points());
	}
	template<size_t K_half>
	inline ContactResult contacts(const KDOP<K_half>& c1, const Circle& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_contacts_polygon(c2, c1, c1.points()).invert();
	}
	template<size_t K_half>
	inline ContactResult contacts(const Circle& c1, const KDOP<K_half>& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_contacts_polygon(c1, c2, c2.points());
	}

	// ######################################################################################################################################################

	// ######################################################################################################################################################
	// KDOP - Rest
	
	// TODO only do SAT if shapes have low enough degree. Otherwise, use GJK
#define OLY_KDOP_COLLISION_METHODS(Shape)\
	template<size_t K_half>\
	inline OverlapResult overlaps(const KDOP<K_half>& c1, const Shape& c2)\
	{\
		return sat::overlaps(c1, c2);\
	}\
	template<size_t K_half>\
	inline OverlapResult overlaps(const Shape& c1, const KDOP<K_half>& c2)\
	{\
		return sat::overlaps(c1, c2);\
	}\
	template<size_t K_half>\
	inline CollisionResult collides(const KDOP<K_half>& c1, const Shape& c2)\
	{\
		return sat::collides(c1, c2);\
	}\
	template<size_t K_half>\
	inline CollisionResult collides(const Shape& c1, const KDOP<K_half>& c2)\
	{\
		return sat::collides(c1, c2);\
	}\
	template<size_t K_half>\
	inline ContactResult contacts(const KDOP<K_half>& c1, const Shape& c2)\
	{\
		return sat::contacts(c1, c2);\
	}\
	template<size_t K_half>\
	inline ContactResult contacts(const Shape& c1, const KDOP<K_half>& c2)\
	{\
		return sat::contacts(c1, c2);\
	}

	OLY_KDOP_COLLISION_METHODS(AABB);
	OLY_KDOP_COLLISION_METHODS(OBB);
	OLY_KDOP_COLLISION_METHODS(ConvexHull);
	OLY_KDOP_COLLISION_METHODS(CustomKDOP);
#undef OLY_KDOP_COLLISION_METHODS

	// ######################################################################################################################################################

	// ######################################################################################################################################################
	// CustomKDOP - Circle
	
	inline OverlapResult overlaps(const CustomKDOP& c1, const Circle& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_overlaps_polygon(c2, c1.points());
	}
	inline OverlapResult overlaps(const Circle& c1, const CustomKDOP& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_overlaps_polygon(c1, c2.points());
	}
	inline CollisionResult collides(const CustomKDOP& c1, const Circle& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_collides_polygon(c2, c1.points()).invert();
	}
	inline CollisionResult collides(const Circle& c1, const CustomKDOP& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_collides_polygon(c1, c2.points());
	}
	inline ContactResult contacts(const CustomKDOP& c1, const Circle& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_contacts_polygon(c2, c1, c1.points()).invert();
	}
	inline ContactResult contacts(const Circle& c1, const CustomKDOP& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return internal::circle_contacts_polygon(c1, c2, c2.points());
	}

	// ######################################################################################################################################################

	// ######################################################################################################################################################
	// CustomKDOP - Rest
	
	// TODO only do SAT if shapes have low enough degree. Otherwise, use GJK
#define OLY_CUSTOM_KDOP_COLLISION_METHODS(Shape)\
	inline OverlapResult overlaps(const CustomKDOP& c1, const Shape& c2)\
	{\
		return sat::overlaps(c1, c2);\
	}\
	inline OverlapResult overlaps(const Shape& c1, const CustomKDOP& c2)\
	{\
		return sat::overlaps(c1, c2);\
	}\
	inline CollisionResult collides(const CustomKDOP& c1, const Shape& c2)\
	{\
		return sat::collides(c1, c2);\
	}\
	inline CollisionResult collides(const Shape& c1, const CustomKDOP& c2)\
	{\
		return sat::collides(c1, c2);\
	}\
	inline ContactResult contacts(const CustomKDOP& c1, const Shape& c2)\
	{\
		return sat::contacts(c1, c2);\
	}\
	inline ContactResult contacts(const Shape& c1, const CustomKDOP& c2)\
	{\
		return sat::contacts(c1, c2);\
	}\

	OLY_CUSTOM_KDOP_COLLISION_METHODS(AABB);
	OLY_CUSTOM_KDOP_COLLISION_METHODS(OBB);
	OLY_CUSTOM_KDOP_COLLISION_METHODS(ConvexHull);
#undef OLY_CUSTOM_KDOP_COLLISION_METHODS
	// ######################################################################################################################################################
}
