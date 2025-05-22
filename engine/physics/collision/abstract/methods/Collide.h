#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"

#include "physics/collision/abstract/primitives/ConvexHull.h"
#include "physics/collision/abstract/primitives/AABB.h"
#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/primitives/OBB.h"
#include "physics/collision/abstract/primitives/KDOP.h"

namespace oly::acm2d
{
	// Circle
	extern OverlapResult point_hits(const Circle& c, glm::vec2 test);
	extern OverlapResult ray_hits(const Circle& c, Ray ray);
	extern RaycastResult raycast(const Circle& c, Ray ray);
	extern OverlapResult overlaps(const Circle& c1, const Circle& c2);
	extern CollisionResult collides(const Circle& c1, const Circle& c2);
	extern ContactResult contacts(const Circle& c1, const Circle& c2);

	// AABB
	extern OverlapResult point_hits(const AABB& c, glm::vec2 test);
	extern OverlapResult ray_hits(const AABB& c, Ray ray);
	extern RaycastResult raycast(const AABB& c, Ray ray);
	extern OverlapResult overlaps(const AABB& c1, const AABB& c2);
	extern CollisionResult collides(const AABB& c1, const AABB& c2);
	extern ContactResult contacts(const AABB& c1, const AABB& c2);
	
	// OBB
	extern OverlapResult point_hits(const OBB& c, glm::vec2 test);
	extern OverlapResult ray_hits(const OBB& c, Ray ray);
	extern RaycastResult raycast(const OBB& c, Ray ray);
	extern OverlapResult overlaps(const OBB& c1, const OBB& c2);
	extern CollisionResult collides(const OBB& c1, const OBB& c2);
	extern ContactResult contacts(const OBB& c1, const OBB& c2);

	// Compound
	extern OverlapResult overlaps(const Circle& c1, const AABB& c2);
	inline OverlapResult overlaps(const AABB& c1, const Circle& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const Circle& c1, const AABB& c2);
	inline CollisionResult collides(const AABB& c1, const Circle& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const Circle& c1, const AABB& c2);
	inline ContactResult contacts(const AABB& c1, const Circle& c2) { return contacts(c2, c1).invert(); }
}
