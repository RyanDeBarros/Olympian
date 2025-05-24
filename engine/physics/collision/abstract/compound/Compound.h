#pragma once

#include "physics/collision/abstract/methods/Collide.h"
#include "physics/collision/abstract/methods/KDOPCollide.h"
#include "core/base/Transforms.h"

namespace oly::acm2d
{
	using KDOP6 = KDOP<6>;
	using KDOP8 = KDOP<8>;
	using KDOP10 = KDOP<10>;
	using KDOP12 = KDOP<12>;
	using KDOP14 = KDOP<14>;
	using KDOP16 = KDOP<16>;

	using Primitive = std::variant<
		Circle,
		AABB,
		OBB,
		ConvexHull,
		// TODO add CustomKDOP which is not templated
		KDOP6,
		KDOP8,
		KDOP10,
		KDOP12,
		KDOP14,
		KDOP16
	>;

	struct Compound
	{
		std::vector<Primitive> primitives;
	};

	extern OverlapResult point_hits(const Compound& c, glm::vec2 test);
	extern OverlapResult ray_hits(const Compound& c, const Ray& ray);
	extern RaycastResult raycast(const Compound& c, const Ray& ray);
	extern OverlapResult overlaps(const Compound& c1, const Compound& c2);
	extern CollisionResult collides(const Compound& c1, const Compound& c2);
	extern ContactResult contacts(const Compound& c1, const Compound& c2);

	struct TCompound
	{
		Compound compound;
		Transformer2D transformer;
	};
}
