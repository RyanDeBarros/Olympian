#pragma once

#include "physics/collision/abstract/methods/Collide.h"
#include "physics/collision/abstract/methods/KDOPCollide.h"
#include "core/base/Transforms.h"

namespace oly::acm2d
{
	extern CollisionResult greedy_collision(const std::vector<CollisionResult>& collisions);
	extern ContactResult greedy_collision(const std::vector<ContactResult>& contacts);

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
		CustomKDOP,
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

	extern OverlapResult overlaps(const Compound& c1, const Primitive& c2);
	inline OverlapResult overlaps(const Primitive& c1, const Compound& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const Compound& c1, const Primitive& c2);
	inline CollisionResult collides(const Primitive& c1, const Compound& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const Compound& c1, const Primitive& c2);
	inline ContactResult contacts(const Primitive& c1, const Compound& c2) { return contacts(c2, c1).invert(); }

	struct TCompound
	{
		Compound compound;
		Transformer2D transformer;
	};

	extern OverlapResult point_hits(const TCompound& c, glm::vec2 test);
	extern OverlapResult ray_hits(const TCompound& c, const Ray& ray);
	extern RaycastResult raycast(const TCompound& c, const Ray& ray);
	extern OverlapResult overlaps(const TCompound& c1, const TCompound& c2);
	extern CollisionResult collides(const TCompound& c1, const TCompound& c2);
	extern ContactResult contacts(const TCompound& c1, const TCompound& c2);

	extern OverlapResult overlaps(const TCompound& c1, const Compound& c2);
	inline OverlapResult overlaps(const Compound& c1, const TCompound& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const TCompound& c1, const Compound& c2);
	inline CollisionResult collides(const Compound& c1, const TCompound& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const TCompound& c1, const Compound& c2);
	inline ContactResult contacts(const Compound& c1, const TCompound& c2) { return contacts(c2, c1).invert(); }

	extern OverlapResult overlaps(const TCompound& c1, const Primitive& c2);
	inline OverlapResult overlaps(const Primitive& c1, const TCompound& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const TCompound& c1, const Primitive& c2);
	inline CollisionResult collides(const Primitive& c1, const TCompound& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const TCompound& c1, const Primitive& c2);
	inline ContactResult contacts(const Primitive& c1, const TCompound& c2) { return contacts(c2, c1).invert(); }
}
