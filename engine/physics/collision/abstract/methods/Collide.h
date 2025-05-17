#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"

#include "physics/collision/abstract/primitives/ConvexHull.h"
#include "physics/collision/abstract/primitives/AABB.h"
#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/primitives/OBB.h"
#include "physics/collision/abstract/primitives/KDOP.h"

namespace oly::acm2d
{
	// circle
	extern bool point_trace(const Circle& c, glm::vec2 test);
	extern OverlapInfo ray_trace(const Circle& c, Ray ray);
	extern SimpleRayHit simple_ray_trace(const Circle& c, Ray ray);
	extern DeepRayHit deep_ray_trace(const Circle& c, Ray ray);
	extern OverlapInfo overlap(const Circle& c1, const Circle& c2);
	extern GeometricInfo geometric_collision(const Circle& c1, const Circle& c2);
	extern StructuralInfo structural_collision(const Circle& c1, const Circle& c2);

	// AABB
	extern bool point_trace(const AABB& c, glm::vec2 test);
	extern OverlapInfo overlap(const AABB& c1, const AABB& c2);
	extern GeometricInfo geometric_collision(const AABB& c1, const AABB& c2);
	extern StructuralInfo structural_collision(const AABB& c1, const AABB& c2);
}
