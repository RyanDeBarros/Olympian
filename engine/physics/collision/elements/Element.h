#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/elements/Circle.h"
#include "physics/collision/elements/AABB.h"
#include "physics/collision/elements/OBB.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/KDOP.h"
#include "physics/collision/Tolerance.h"

#include "core/types/CopyPtr.h"

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
		CopyPtr<CustomKDOP>,
		CopyPtr<KDOP6>,
		CopyPtr<KDOP8>,
		CopyPtr<KDOP10>,
		CopyPtr<KDOP12>,
		CopyPtr<KDOP14>,
		CopyPtr<KDOP16>
	>;

	extern Element transform_element(const Circle& c, const glm::mat3& m);
	extern Element transform_element(const AABB& c, const glm::mat3& m);
	extern Element transform_element(const OBB& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<CustomKDOP>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP6>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP8>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP10>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP12>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP14>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP16>& c, const glm::mat3& m);
	extern Element transform_element(const ConvexHull& c, const glm::mat3& m);

	typedef int Mask;
	typedef int Layer;
}
