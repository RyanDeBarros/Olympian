#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"
#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/primitives/AABB.h"
#include "physics/collision/abstract/primitives/OBB.h"

namespace oly::acm2d::sat
{
	// Convex polygon
	extern OverlapResult overlaps(const std::vector<glm::vec2>& c1, const std::vector<glm::vec2>& c2);
	extern CollisionResult collides(const std::vector<glm::vec2>& c1, const std::vector<glm::vec2>& c2);

	// Circle
	extern OverlapResult overlaps(const Circle& c1, const std::vector<glm::vec2>& c2);
	extern CollisionResult collides(const Circle& c1, const std::vector<glm::vec2>& c2);
	inline OverlapResult overlaps(const std::vector<glm::vec2>& c1, const Circle& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const std::vector<glm::vec2>& c1, const Circle& c2) { return collides(c2, c1).invert(); }

	// AABB
	extern OverlapResult overlaps(const AABB& c1, const std::vector<glm::vec2>& c2);
	extern CollisionResult collides(const AABB& c1, const std::vector<glm::vec2>& c2);
	inline OverlapResult overlaps(const std::vector<glm::vec2>& c1, const AABB& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const std::vector<glm::vec2>& c1, const AABB& c2) { return collides(c2, c1).invert(); }

	// OBB
	extern OverlapResult overlaps(const OBB& c1, const std::vector<glm::vec2>& c2);
	extern CollisionResult collides(const OBB& c1, const std::vector<glm::vec2>& c2);
	inline OverlapResult overlaps(const std::vector<glm::vec2>& c1, const OBB& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const std::vector<glm::vec2>& c1, const OBB& c2) { return collides(c2, c1).invert(); }
}
