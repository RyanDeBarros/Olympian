#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"

#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/primitives/AABB.h"
#include "physics/collision/abstract/primitives/OBB.h"
#include "physics/collision/abstract/primitives/ConvexHull.h"
#include "physics/collision/abstract/primitives/Capsule.h"
#include "physics/collision/abstract/primitives/HalfPlane.h"

namespace oly::acm2d
{
	namespace internal
	{
		extern OverlapResult ray_hits_slab(float min_proj, float max_proj, const Ray& ray, const UnitVector2D& axis);
		extern bool raycast_update_on_slab(float min_proj, float max_proj, const Ray& ray, const UnitVector2D& axis, RaycastResult& info, float& max_entry);
	}

	// Matched

	// Circle
	extern OverlapResult point_hits(const Circle& c, glm::vec2 test);
	extern OverlapResult ray_hits(const Circle& c, const Ray& ray);
	extern RaycastResult raycast(const Circle& c, const Ray& ray);
	extern OverlapResult overlaps(const Circle& c1, const Circle& c2);
	extern CollisionResult collides(const Circle& c1, const Circle& c2);
	extern ContactResult contacts(const Circle& c1, const Circle& c2);

	// AABB
	extern OverlapResult point_hits(const AABB& c, glm::vec2 test);
	extern OverlapResult ray_hits(const AABB& c, const Ray& ray);
	extern RaycastResult raycast(const AABB& c, const Ray& ray);
	extern OverlapResult overlaps(const AABB& c1, const AABB& c2);
	extern CollisionResult collides(const AABB& c1, const AABB& c2);
	extern ContactResult contacts(const AABB& c1, const AABB& c2);
	
	// OBB
	extern OverlapResult point_hits(const OBB& c, glm::vec2 test);
	extern OverlapResult ray_hits(const OBB& c, const Ray& ray);
	extern RaycastResult raycast(const OBB& c, const Ray& ray);
	extern OverlapResult overlaps(const OBB& c1, const OBB& c2);
	extern CollisionResult collides(const OBB& c1, const OBB& c2);
	extern ContactResult contacts(const OBB& c1, const OBB& c2);

	// ConvexHull
	extern OverlapResult point_hits(const ConvexHull& c, glm::vec2 test);
	extern OverlapResult ray_hits(const ConvexHull& c, const Ray& ray);
	extern RaycastResult raycast(const ConvexHull& c, const Ray& ray);
	extern OverlapResult overlaps(const ConvexHull& c1, const ConvexHull& c2);
	extern CollisionResult collides(const ConvexHull& c1, const ConvexHull& c2);
	extern ContactResult contacts(const ConvexHull& c1, const ConvexHull& c2);

	// Mixed
	
	// Circle - AABB
	extern OverlapResult overlaps(const Circle& c1, const AABB& c2);
	inline OverlapResult overlaps(const AABB& c1, const Circle& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const Circle& c1, const AABB& c2);
	inline CollisionResult collides(const AABB& c1, const Circle& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const Circle& c1, const AABB& c2);
	inline ContactResult contacts(const AABB& c1, const Circle& c2) { return contacts(c2, c1).invert(); }

	// Circle - OBB
	extern OverlapResult overlaps(const Circle& c1, const OBB& c2);
	inline OverlapResult overlaps(const OBB& c1, const Circle& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const Circle& c1, const OBB& c2);
	inline CollisionResult collides(const OBB& c1, const Circle& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const Circle& c1, const OBB& c2);
	inline ContactResult contacts(const OBB& c1, const Circle& c2) { return contacts(c2, c1).invert(); }

	// AABB - OBB
	extern OverlapResult overlaps(const AABB& c1, const OBB& c2);
	inline OverlapResult overlaps(const OBB& c1, const AABB& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const AABB& c1, const OBB& c2);
	inline CollisionResult collides(const OBB& c1, const AABB& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const AABB& c1, const OBB& c2);
	inline ContactResult contacts(const OBB& c1, const AABB& c2) { return contacts(c2, c1).invert(); }

	// Circle - ConvexHull
	extern OverlapResult overlaps(const Circle& c1, const ConvexHull& c2);
	inline OverlapResult overlaps(const ConvexHull& c1, const Circle& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const Circle& c1, const ConvexHull& c2);
	inline CollisionResult collides(const ConvexHull& c1, const Circle& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const Circle& c1, const ConvexHull& c2);
	inline ContactResult contacts(const ConvexHull& c1, const Circle& c2) { return contacts(c2, c1).invert(); }

	// ConvexHull - AABB
	extern OverlapResult overlaps(const ConvexHull& c1, const AABB& c2);
	inline OverlapResult overlaps(const AABB& c1, const ConvexHull& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const ConvexHull& c1, const AABB& c2);
	inline CollisionResult collides(const AABB& c1, const ConvexHull& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const ConvexHull& c1, const AABB& c2);
	inline ContactResult contacts(const AABB& c1, const ConvexHull& c2) { return contacts(c2, c1).invert(); }

	// ConvexHull - OBB
	extern OverlapResult overlaps(const ConvexHull& c1, const OBB& c2);
	inline OverlapResult overlaps(const OBB& c1, const ConvexHull& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const ConvexHull& c1, const OBB& c2);
	inline CollisionResult collides(const OBB& c1, const ConvexHull& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const ConvexHull& c1, const OBB& c2);
	inline ContactResult contacts(const OBB& c1, const ConvexHull& c2) { return contacts(c2, c1).invert(); }
}
