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
	using KDOP3 = KDOP<3>;
	using KDOP4 = KDOP<4>;
	using KDOP5 = KDOP<5>;
	using KDOP6 = KDOP<6>;
	using KDOP7 = KDOP<7>;
	using KDOP8 = KDOP<8>;

	using Element = std::variant<
		Circle,
		AABB,
		OBB,
		ConvexHull,
		CopyPtr<CustomKDOP>,
		CopyPtr<KDOP3>,
		CopyPtr<KDOP4>,
		CopyPtr<KDOP5>,
		CopyPtr<KDOP6>,
		CopyPtr<KDOP7>,
		CopyPtr<KDOP8>
	>;

	extern Element transform_element(const Circle& c, const glm::mat3& m);
	extern Element transform_element(const AABB& c, const glm::mat3& m);
	extern Element transform_element(const OBB& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<CustomKDOP>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP3>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP4>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP5>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP6>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP7>& c, const glm::mat3& m);
	extern Element transform_element(const CopyPtr<KDOP8>& c, const glm::mat3& m);
	extern Element transform_element(const ConvexHull& c, const glm::mat3& m);

	typedef int Mask;
	typedef int Layer;
}
