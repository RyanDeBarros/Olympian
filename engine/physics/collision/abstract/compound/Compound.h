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

	extern Primitive transform_primitive(const Circle& c, const glm::mat3& m);
	extern Primitive transform_primitive(const AABB& c, const glm::mat3& m);
	extern Primitive transform_primitive(const OBB& c, const glm::mat3& m);
	extern Primitive transform_primitive(const CustomKDOP& c, const glm::mat3& m);
	extern Primitive transform_primitive(const KDOP6& c, const glm::mat3& m);
	extern Primitive transform_primitive(const KDOP8& c, const glm::mat3& m);
	extern Primitive transform_primitive(const KDOP10& c, const glm::mat3& m);
	extern Primitive transform_primitive(const KDOP12& c, const glm::mat3& m);
	extern Primitive transform_primitive(const KDOP14& c, const glm::mat3& m);
	extern Primitive transform_primitive(const KDOP16& c, const glm::mat3& m);
	extern Primitive transform_primitive(const ConvexHull& c, const glm::mat3& m);

	typedef int Mask;
	typedef int Layer;
	struct Compound
	{
		std::vector<Primitive> primitives;

		Mask mask;
		Layer layer;
	};

	// TODO collision tests involving 2 compounds should check for interaction first (active.mask & static.layer) and then call internal _test methods that don't check for interaction.

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

	class TCompound
	{
		Compound compound;
		Transformer2D transformer;
		mutable std::vector<Primitive> baked;
		mutable bool dirty;

		void bake() const;

	public:
		glm::mat3 global() const { return transformer.global(); }

		// TODO interface for attaching/detaching transformer to hierarchies, without exposing it directly

		const Compound& get_compound() const { return compound; }
		Compound& set_compound() { dirty = true; return compound; }
		const std::vector<Primitive>& get_baked() const { if (dirty) { dirty = false; bake(); } return baked; }
		// TODO bake transformed primitives. Therefore, set_local() and set_compound() should refresh these baked primitives (or rather flag them for baking - get_world_compound() actually bakes them if flagged), so that they don't need to be transformed in collision tests.
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

	// TODO PointCloud that can be transformed and can bake a BVH. When using collision in game, try to use PointCloud if possible.
}
