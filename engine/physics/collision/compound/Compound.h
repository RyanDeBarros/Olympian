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

	struct Primitive
	{
		Element element;

		Mask mask = 0;
		Layer layer = 0;
	};

	inline OverlapResult point_hits(const Primitive& c, glm::vec2 test) { return std::visit([test](auto&& e) { return point_hits(e, test); }, c.element); }
	inline OverlapResult ray_hits(const Primitive& c, const Ray& ray) { return std::visit([&ray](auto&& e) { return ray_hits(e, ray); }, c.element); }
	inline RaycastResult raycast(const Primitive& c, const Ray& ray) { return std::visit([&ray](auto&& e) { return raycast(e, ray); }, c.element); }
	inline OverlapResult overlaps(const Primitive& c1, const Primitive& c2)
	{
		return (c1.mask & c2.layer) && std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return overlaps(e1, e2);
				}, c2.element);
			}, c1.element);
	}
	inline CollisionResult collides(const Primitive& c1, const Primitive& c2)
	{
		return (c1.mask & c2.layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return collides(e1, e2);
				}, c2.element);
			}, c1.element) : CollisionResult{ .overlap = false };
	}
	inline ContactResult contacts(const Primitive& c1, const Primitive& c2)
	{
		return (c1.mask & c2.layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return contacts(e1, e2);
				}, c2.element);
			}, c1.element) : ContactResult{ .overlap = false };
	}

	class TPrimitive
	{
		Primitive primitive;
		mutable Element baked;
		mutable bool dirty;

		void bake() const;

	public:
		Transformer2D transformer;
		glm::mat3 global() const { return transformer.global(); }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { flag(); return transformer.set_local(); }

		void flag() const { dirty = true; }
		const Primitive& get_primitive() const { return primitive; }
		Primitive& set_primitive() { flag(); return primitive; }
		const Element& get_baked() const { if (dirty) { bake(); } return baked; }
	};

	inline OverlapResult point_hits(const TPrimitive& c, glm::vec2 test) { return std::visit([test](auto&& e) { return point_hits(e, test); }, c.get_baked()); }
	inline OverlapResult ray_hits(const TPrimitive& c, const Ray& ray) { return std::visit([&ray](auto&& e) { return ray_hits(e, ray); }, c.get_baked()); }
	inline RaycastResult raycast(const TPrimitive& c, const Ray& ray) { return std::visit([&ray](auto&& e) { return raycast(e, ray); }, c.get_baked()); }

	inline OverlapResult overlaps(const TPrimitive& c1, const TPrimitive& c2)
	{
		return (c1.get_primitive().mask & c2.get_primitive().layer) && std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return overlaps(e1, e2);
				}, c2.get_baked());
			}, c1.get_baked());
	}
	inline OverlapResult overlaps(const TPrimitive& c1, const Primitive& c2)
	{
		return (c1.get_primitive().mask & c2.layer) && std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return overlaps(e1, e2);
				}, c2.element);
			}, c1.get_baked());
	}
	inline OverlapResult overlaps(const Primitive& c1, const TPrimitive& c2)
	{
		return (c1.mask & c2.get_primitive().layer) && std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return overlaps(e1, e2);
				}, c2.get_baked());
			}, c1.element);
	}
	inline CollisionResult collides(const TPrimitive& c1, const TPrimitive& c2)
	{
		return (c1.get_primitive().mask & c2.get_primitive().layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return collides(e1, e2);
				}, c2.get_baked());
			}, c1.get_baked()) : CollisionResult{ .overlap = false };
	}
	inline CollisionResult collides(const TPrimitive& c1, const Primitive& c2)
	{
		return (c1.get_primitive().mask & c2.layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return collides(e1, e2);
				}, c2.element);
			}, c1.get_baked()) : CollisionResult{ .overlap = false };
	}
	inline CollisionResult collides(const Primitive& c1, const TPrimitive& c2)
	{
		return (c1.mask & c2.get_primitive().layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return collides(e1, e2);
				}, c2.get_baked());
			}, c1.element) : CollisionResult{ .overlap = false };
	}
	inline ContactResult contacts(const TPrimitive& c1, const TPrimitive& c2)
	{
		return (c1.get_primitive().mask & c2.get_primitive().layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return contacts(e1, e2);
				}, c2.get_baked());
			}, c1.get_baked()) : ContactResult{ .overlap = false };
	}
	inline ContactResult contacts(const TPrimitive& c1, const Primitive& c2)
	{
		return (c1.get_primitive().mask & c2.layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return contacts(e1, e2);
				}, c2.element);
			}, c1.get_baked()) : ContactResult{ .overlap = false };
	}
	inline ContactResult contacts(const Primitive& c1, const TPrimitive& c2)
	{
		return (c1.mask & c2.get_primitive().layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return contacts(e1, e2);
				}, c2.get_baked());
			}, c1.element) : ContactResult{ .overlap = false };
	}

	struct Compound
	{
		std::vector<Element> elements;

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

	extern OverlapResult overlaps(const Compound& c1, const Element& c2);
	inline OverlapResult overlaps(const Element& c1, const Compound& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const Compound& c1, const Element& c2);
	inline CollisionResult collides(const Element& c1, const Compound& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const Compound& c1, const Element& c2);
	inline ContactResult contacts(const Element& c1, const Compound& c2) { return contacts(c2, c1).invert(); }

	class TCompound
	{
		Compound compound;
		mutable std::vector<Element> baked;
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
		const std::vector<Element>& get_baked() const { if (dirty) { bake(); } return baked; }
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

	extern OverlapResult overlaps(const TCompound& c1, const Element& c2);
	inline OverlapResult overlaps(const Element& c1, const TCompound& c2) { return overlaps(c2, c1); }
	extern CollisionResult collides(const TCompound& c1, const Element& c2);
	inline CollisionResult collides(const Element& c1, const TCompound& c2) { return collides(c2, c1).invert(); }
	extern ContactResult contacts(const TCompound& c1, const Element& c2);
	inline ContactResult contacts(const Element& c1, const TCompound& c2) { return contacts(c2, c1).invert(); }

	inline OverlapResult overlaps(const Compound& c1, const Primitive& c2) { return (c1.mask & c2.layer) && overlaps(c1, c2.element); }
	inline OverlapResult overlaps(const Primitive& c1, const Compound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const Compound& c1, const Primitive& c2) { return (c1.mask & c2.layer) ? collides(c1, c2.element) : CollisionResult{.overlap = false }; }
	inline CollisionResult collides(const Primitive& c1, const Compound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const Compound& c1, const Primitive& c2) { return (c1.mask & c2.layer) ? contacts(c1, c2.element) : ContactResult{ .overlap = false }; }
	inline ContactResult contacts(const Primitive& c1, const Compound& c2) { return contacts(c2, c1).invert(); }

	inline OverlapResult overlaps(const TCompound& c1, const Primitive& c2) { return (c1.get_compound().mask & c2.layer) && overlaps(c1, c2.element); }
	inline OverlapResult overlaps(const Primitive& c1, const TCompound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const TCompound& c1, const Primitive& c2) { return (c1.get_compound().mask & c2.layer) ? collides(c1, c2.element) : CollisionResult{ .overlap = false }; }
	inline CollisionResult collides(const Primitive& c1, const TCompound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const TCompound& c1, const Primitive& c2) { return (c1.get_compound().mask & c2.layer) ? contacts(c1, c2.element) : ContactResult{ .overlap = false }; }
	inline ContactResult contacts(const Primitive& c1, const TCompound& c2) { return contacts(c2, c1).invert(); }

	inline OverlapResult overlaps(const Compound& c1, const TPrimitive& c2) { return (c1.mask & c2.get_primitive().layer) && overlaps(c1, c2.get_baked()); }
	inline OverlapResult overlaps(const TPrimitive& c1, const Compound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const Compound& c1, const TPrimitive& c2) { return (c1.mask & c2.get_primitive().layer) ? collides(c1, c2.get_baked()) : CollisionResult{.overlap = false}; }
	inline CollisionResult collides(const TPrimitive& c1, const Compound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const Compound& c1, const TPrimitive& c2) { return (c1.mask & c2.get_primitive().layer) ? contacts(c1, c2.get_baked()) : ContactResult{.overlap = false}; }
	inline ContactResult contacts(const TPrimitive& c1, const Compound& c2) { return contacts(c2, c1).invert(); }

	inline OverlapResult overlaps(const TCompound& c1, const TPrimitive& c2) { return (c1.get_compound().mask & c2.get_primitive().layer) && overlaps(c1, c2.get_baked()); }
	inline OverlapResult overlaps(const TPrimitive& c1, const TCompound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const TCompound& c1, const TPrimitive& c2) { return (c1.get_compound().mask & c2.get_primitive().layer) ? collides(c1, c2.get_baked()) : CollisionResult{ .overlap = false }; }
	inline CollisionResult collides(const TPrimitive& c1, const TCompound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const TCompound& c1, const TPrimitive& c2) { return (c1.get_compound().mask & c2.get_primitive().layer) ? contacts(c1, c2.get_baked()) : ContactResult{ .overlap = false }; }
	inline ContactResult contacts(const TPrimitive& c1, const TCompound& c2) { return contacts(c2, c1).invert(); }
}
