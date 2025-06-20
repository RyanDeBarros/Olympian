#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/methods/SAT.h"
#include "physics/collision/methods/GJK.h"
#include "physics/collision/methods/CircleMethods.h"

#include "physics/collision/elements/KDOP.h"
#include "physics/collision/elements/Circle.h"
#include "physics/collision/elements/AABB.h"
#include "physics/collision/elements/OBB.h"
#include "physics/collision/elements/ConvexHull.h"

#include "physics/collision/Tolerance.h"

namespace oly::col2d
{
	// Matched

	// ######################################################################################################################################################
	// KDOP

	template<size_t K>
	inline OverlapResult point_hits(const KDOP<K>& c, glm::vec2 test)
	{
		for (size_t i = 0; i < K; ++i)
		{
			float proj = KDOP<K>::uniform_axis(i).dot(test);
			if (proj < c.get_clipped_minimum(i) || proj > c.get_clipped_maximum(i))
				return false;
		}
		return true;
	}
	
	template<size_t K>
	inline OverlapResult ray_hits(const KDOP<K>& c, const Ray& ray)
	{
		for (size_t i = 0; i < K; ++i)
		{
			if (!internal::ray_hits_slab(c.get_clipped_minimum(i), c.get_clipped_maximum(i), ray, KDOP<K>::uniform_axis(i)))
				return false;
		}
		return true;
	}
	
	template<size_t K>
	inline RaycastResult raycast(const KDOP<K>& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::EMBEDDED_ORIGIN, .contact = ray.origin };
		float max_entry = -nmax<float>();
		for (size_t i = 0; i < K; ++i)
		{
			if (!internal::raycast_update_on_slab(c.get_clipped_minimum(i), c.get_clipped_maximum(i), ray, KDOP<K>::uniform_axis(i), info, max_entry))
				return { .hit = RaycastResult::Hit::NO_HIT };
		}
		if (info.hit == RaycastResult::Hit::TRUE_HIT)
			info.contact = ray.origin + max_entry * (glm::vec2)ray.direction;
		return info;
	}
	
	template<size_t K1, size_t K2>
	inline OverlapResult overlaps(const KDOP<K1>& c1, const KDOP<K2>& c2)
	{
		if (2 * (K1 + K2) >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::overlaps(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW)
					throw e;
			}
		}
		return sat::overlaps(c1, c2);
	}
	
	template<size_t K1, size_t K2>
	inline CollisionResult collides(const KDOP<K1>& c1, const KDOP<K2>& c2)
	{
		if (2 * (K1 + K2) >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::collides(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return sat::collides(c1, c2);
	}
	
	template<size_t K1, size_t K2>
	inline ContactResult contacts(const KDOP<K1>& c1, const KDOP<K2>& c2)
	{
		if (2 * (K1 + K2) >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::contacts(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return sat::contacts(c1, c2);
	}

	// ######################################################################################################################################################

	// Mixed

	// ######################################################################################################################################################
	// KDOP - Circle

	template<size_t K>
	inline OverlapResult overlaps(const KDOP<K>& c1, const Circle& c2)
	{
		if (2 * K + 1 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::overlaps(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW)
					throw e;
			}
		}
		return internal::circle_overlaps_polygon(c2, c1.points());
	}

	template<size_t K>
	inline OverlapResult overlaps(const Circle& c1, const KDOP<K>& c2)
	{
		if (2 * K + 1 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::overlaps(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW)
					throw e;
			}
		}
		return internal::circle_overlaps_polygon(c1, c2.points());
	}

	template<size_t K>
	inline CollisionResult collides(const KDOP<K>& c1, const Circle& c2)
	{
		if (2 * K + 1 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::collides(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return internal::circle_collides_polygon(c2, c1.points()).invert();
	}

	template<size_t K>
	inline CollisionResult collides(const Circle& c1, const KDOP<K>& c2)
	{
		if (2 * K + 1 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::collides(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return internal::circle_collides_polygon(c1, c2.points());
	}

	template<size_t K>
	inline ContactResult contacts(const KDOP<K>& c1, const Circle& c2)
	{
		if (2 * K + 1 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::contacts(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return internal::circle_contacts_polygon(c2, c1, c1.points()).invert();
	}

	template<size_t K>
	inline ContactResult contacts(const Circle& c1, const KDOP<K>& c2)
	{
		if (2 * K + 1 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::contacts(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return internal::circle_contacts_polygon(c1, c2, c2.points());
	}

	// ######################################################################################################################################################

	// ######################################################################################################################################################
	// KDOP - Rest
	
#define OLY_KDOP_COLLISION_METHODS(Shape, shape_vertices1, shape_vertices2)\
	template<size_t K>\
	inline OverlapResult overlaps(const KDOP<K>& c1, const Shape& c2)\
	{\
		if (2 * K + shape_vertices2 >= gjk::VERTICES_THRESHOLD)\
		{\
			try\
			{\
				return gjk::overlaps(c1, c2);\
			}\
			catch (Error e)\
			{\
				if (e.code != ErrorCode::GJK_OVERFLOW)\
					throw e;\
			}\
		}\
		return sat::overlaps(c1, c2);\
	}\
	template<size_t K>\
	inline OverlapResult overlaps(const Shape& c1, const KDOP<K>& c2)\
	{\
		if (2 * K + shape_vertices1 >= gjk::VERTICES_THRESHOLD)\
		{\
			try\
			{\
				return gjk::overlaps(c1, c2);\
			}\
			catch (Error e)\
			{\
				if (e.code != ErrorCode::GJK_OVERFLOW)\
					throw e;\
			}\
		}\
		return sat::overlaps(c1, c2);\
	}\
	template<size_t K>\
	inline CollisionResult collides(const KDOP<K>& c1, const Shape& c2)\
	{\
		if (2 * K + shape_vertices2 >= gjk::VERTICES_THRESHOLD)\
		{\
			try\
			{\
				return gjk::collides(c1, c2);\
			}\
			catch (Error e)\
			{\
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)\
					throw e;\
			}\
		}\
		return sat::collides(c1, c2);\
	}\
	template<size_t K>\
	inline CollisionResult collides(const Shape& c1, const KDOP<K>& c2)\
	{\
		if (2 * K + shape_vertices1 >= gjk::VERTICES_THRESHOLD)\
		{\
			try\
			{\
				return gjk::collides(c1, c2);\
			}\
			catch (Error e)\
			{\
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)\
					throw e;\
			}\
		}\
		return sat::collides(c1, c2);\
	}\
	template<size_t K>\
	inline ContactResult contacts(const KDOP<K>& c1, const Shape& c2)\
	{\
		if (2 * K + shape_vertices2 >= gjk::VERTICES_THRESHOLD)\
		{\
			try\
			{\
				return gjk::contacts(c1, c2);\
			}\
			catch (Error e)\
			{\
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)\
					throw e;\
			}\
		}\
		return sat::contacts(c1, c2);\
	}\
	template<size_t K>\
	inline ContactResult contacts(const Shape& c1, const KDOP<K>& c2)\
	{\
		if (2 * K + shape_vertices1 >= gjk::VERTICES_THRESHOLD)\
		{\
			try\
			{\
				return gjk::contacts(c1, c2);\
			}\
			catch (Error e)\
			{\
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)\
					throw e;\
			}\
		}\
		return sat::contacts(c1, c2);\
	}

	OLY_KDOP_COLLISION_METHODS(AABB, 4, 4);
	OLY_KDOP_COLLISION_METHODS(OBB, 4, 4);
	OLY_KDOP_COLLISION_METHODS(ConvexHull, c1.points().size(), c2.points().size());
#undef OLY_KDOP_COLLISION_METHODS

	// ######################################################################################################################################################
}
