#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"

#include "physics/collision/abstract/primitives/ConvexHull.h"
#include "physics/collision/abstract/primitives/AABB.h"
#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/primitives/OBB.h"
#include "physics/collision/abstract/primitives/KDOP.h"

namespace oly::acm2d
{
	extern OverlapInfo overlap(const Circle& c1, const Circle& c2);
	extern GeometricInfo geometric_collision(const Circle& c1, const Circle& c2);
	extern StructuralInfo structural_collision(const Circle& c1, const Circle& c2);

	extern OverlapInfo overlap(const AABB& c1, const AABB& c2);
	extern GeometricInfo geometric_collision(const AABB& c1, const AABB& c2);
	extern StructuralInfo structural_collision(const AABB& c1, const AABB& c2);
}
