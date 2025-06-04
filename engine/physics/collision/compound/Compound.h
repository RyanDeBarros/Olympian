#pragma once

#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "core/base/Transforms.h"

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

		Mask mask = 0;
		Layer layer = 0;
	};

	extern OverlapResult point_hits(const Compound& c, glm::vec2 test);
	extern OverlapResult ray_hits(const Compound& c, const Ray& ray);
	extern RaycastResult raycast(const Compound& c, const Ray& ray);

	namespace internal
	{
		extern OverlapResult overlaps(const Compound& c1, const Compound& c2);
		extern CollisionResult collides(const Compound& c1, const Compound& c2);
		extern ContactResult contacts(const Compound& c1, const Compound& c2);
	}

	inline OverlapResult overlaps(const Compound& c1, const Compound& c2)
	{
		if (c1.mask & c2.layer)
			return internal::overlaps(c1, c2);
		else
			return false;
	}
	inline CollisionResult collides(const Compound& c1, const Compound& c2)
	{
		if (c1.mask & c2.layer)
			return internal::collides(c1, c2);
		else
			return { .overlap = false };
	}
	inline ContactResult contacts(const Compound& c1, const Compound& c2)
	{
		if (c1.mask & c2.layer)
			return internal::contacts(c1, c2);
		else
			return { .overlap = false };
	}

	extern OverlapResult overlaps(const Compound& c1, const Primitive& c2);
	inline OverlapResult overlaps(const Primitive& c1, const Compound& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const Compound& c1, const Primitive& c2);
	inline CollisionResult collides(const Primitive& c1, const Compound& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const Compound& c1, const Primitive& c2);
	inline ContactResult contacts(const Primitive& c1, const Compound& c2) { return contacts(c2, c1).invert(); }

	class TCompound
	{
		Compound compound;
		mutable std::vector<Primitive> baked;
		mutable bool dirty;

		void bake() const;

	public:
		Transformer2D transformer;
		glm::mat3 global() const { return transformer.global(); }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { flag(); return transformer.set_local(); }

		void flag() const { dirty = true; }
		const Compound& get_compound() const { return compound; }
		Compound& set_compound() { flag(); return compound; }
		const std::vector<Primitive>& get_baked() const { if (dirty) { bake(); } return baked; }
	};

	extern OverlapResult point_hits(const TCompound& c, glm::vec2 test);
	extern OverlapResult ray_hits(const TCompound& c, const Ray& ray);
	extern RaycastResult raycast(const TCompound& c, const Ray& ray);

	namespace internal
	{
		extern OverlapResult overlaps(const TCompound& c1, const TCompound& c2);
		extern CollisionResult collides(const TCompound& c1, const TCompound& c2);
		extern ContactResult contacts(const TCompound& c1, const TCompound& c2);

		extern OverlapResult overlaps(const TCompound& c1, const Compound& c2);
		inline OverlapResult overlaps(const Compound& c1, const TCompound& c2) { return internal::overlaps(c2, c1); }
		extern CollisionResult collides(const TCompound& c1, const Compound& c2);
		inline CollisionResult collides(const Compound& c1, const TCompound& c2) { return internal::collides(c2, c1).invert(); }
		extern ContactResult contacts(const TCompound& c1, const Compound& c2);
		inline ContactResult contacts(const Compound& c1, const TCompound& c2) { return internal::contacts(c2, c1).invert(); }
	}

	inline OverlapResult overlaps(const TCompound& c1, const TCompound& c2)
	{
		if (c1.get_compound().mask & c2.get_compound().layer)
			return internal::overlaps(c1, c2);
		else
			return false;
	}
	inline CollisionResult collides(const TCompound& c1, const TCompound& c2)
	{
		if (c1.get_compound().mask & c2.get_compound().layer)
			return internal::collides(c1, c2);
		else
			return { .overlap = false };
	}
	inline ContactResult contacts(const TCompound& c1, const TCompound& c2)
	{
		if (c1.get_compound().mask & c2.get_compound().layer)
			return internal::contacts(c1, c2);
		else
			return { .overlap = false };
	}

	inline OverlapResult overlaps(const TCompound& c1, const Compound& c2)
	{
		if (c1.get_compound().mask & c2.layer)
			return internal::overlaps(c1, c2);
		else
			return false;
	}
	inline OverlapResult overlaps(const Compound& c1, const TCompound& c2)
	{
		if (c1.mask & c2.get_compound().layer)
			return internal::overlaps(c1, c2);
		else
			return false;
	}
	inline CollisionResult collides(const TCompound& c1, const Compound& c2)
	{
		if (c1.get_compound().mask & c2.layer)
			return internal::collides(c1, c2);
		else
			return { .overlap = false };
	}
	inline CollisionResult collides(const Compound& c1, const TCompound& c2)
	{
		if (c1.mask & c2.get_compound().layer)
			return internal::collides(c1, c2);
		else
			return { .overlap = false };
	}
	inline ContactResult contacts(const TCompound& c1, const Compound& c2)
	{
		if (c1.get_compound().mask & c2.layer)
			return internal::contacts(c1, c2);
		else
			return { .overlap = false };
	}
	inline ContactResult contacts(const Compound& c1, const TCompound& c2)
	{
		if (c1.mask & c2.get_compound().layer)
			return internal::contacts(c1, c2);
		else
			return { .overlap = false };
	}

	extern OverlapResult overlaps(const TCompound& c1, const Primitive& c2);
	inline OverlapResult overlaps(const Primitive& c1, const TCompound& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const TCompound& c1, const Primitive& c2);
	inline CollisionResult collides(const Primitive& c1, const TCompound& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const TCompound& c1, const Primitive& c2);
	inline ContactResult contacts(const Primitive& c1, const TCompound& c2) { return contacts(c2, c1).invert(); }
}
