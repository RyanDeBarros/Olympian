#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/elements/Circle.h"
#include "physics/collision/elements/AABB.h"
#include "physics/collision/elements/OBB.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/KDOP.h"

namespace oly::col2d
{
	extern CollisionResult greedy_collision(const std::vector<CollisionResult>& collisions);
	extern ContactResult greedy_contact(const std::vector<ContactResult>& contacts);

	using KDOP6 = KDOP<6>;
	using KDOP8 = KDOP<8>;
	using KDOP10 = KDOP<10>;
	using KDOP12 = KDOP<12>;
	using KDOP14 = KDOP<14>;
	using KDOP16 = KDOP<16>;

	using Element = std::variant<
		Circle,
		AABB,
		OBB,
		ConvexHull,
		CustomKDOP,
		KDOP6,
		KDOP8,
		KDOP10,
		KDOP12,
		KDOP14,
		KDOP16
	>;

	extern Element transform_element(const Circle& c, const glm::mat3& m);
	extern Element transform_element(const AABB& c, const glm::mat3& m);
	extern Element transform_element(const OBB& c, const glm::mat3& m);
	extern Element transform_element(const CustomKDOP& c, const glm::mat3& m);
	extern Element transform_element(const KDOP6& c, const glm::mat3& m);
	extern Element transform_element(const KDOP8& c, const glm::mat3& m);
	extern Element transform_element(const KDOP10& c, const glm::mat3& m);
	extern Element transform_element(const KDOP12& c, const glm::mat3& m);
	extern Element transform_element(const KDOP14& c, const glm::mat3& m);
	extern Element transform_element(const KDOP16& c, const glm::mat3& m);
	extern Element transform_element(const ConvexHull& c, const glm::mat3& m);

	typedef int Mask;
	typedef int Layer;
}
