#pragma once

#include "physics/collision/elements/Element.h"
#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "physics/collision/Tolerance.h"
#include "core/base/Transforms.h"

namespace oly::col2d
{
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

		void bake() const
		{
			glm::mat3 g = transformer.global();
			baked = std::visit([&g](auto&& e) { return transform_element(e, g); }, primitive.element);
			dirty = false;
		}

	public:
		Transformer2D transformer;
		glm::mat3 global() const { return transformer.global(); }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { flag(); return transformer.set_local(); }

		void flag() const { dirty = true; }
		const Primitive& get_primitive() const { return primitive; }
		Primitive& set_primitive() { flag(); return primitive; }
		const Element& get_baked() const { if (dirty) { bake(); } return baked; }

		Mask mask() const { return primitive.mask; }
		Mask& mask() { return primitive.mask; }
		Layer layer() const { return primitive.layer; }
		Layer& layer() { return primitive.layer; }
	};

	inline OverlapResult point_hits(const TPrimitive& c, glm::vec2 test) { return std::visit([test](auto&& e) { return point_hits(e, test); }, c.get_baked()); }
	inline OverlapResult ray_hits(const TPrimitive& c, const Ray& ray) { return std::visit([&ray](auto&& e) { return ray_hits(e, ray); }, c.get_baked()); }
	inline RaycastResult raycast(const TPrimitive& c, const Ray& ray) { return std::visit([&ray](auto&& e) { return raycast(e, ray); }, c.get_baked()); }

	inline OverlapResult overlaps(const TPrimitive& c1, const TPrimitive& c2)
	{
		return (c1.mask() & c2.layer()) && std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return overlaps(e1, e2);
				}, c2.get_baked());
			}, c1.get_baked());
	}
	inline OverlapResult overlaps(const TPrimitive& c1, const Primitive& c2)
	{
		return (c1.mask() & c2.layer) && std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return overlaps(e1, e2);
				}, c2.element);
			}, c1.get_baked());
	}
	inline OverlapResult overlaps(const Primitive& c1, const TPrimitive& c2)
	{
		return (c1.mask & c2.layer()) && std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return overlaps(e1, e2);
				}, c2.get_baked());
			}, c1.element);
	}
	inline CollisionResult collides(const TPrimitive& c1, const TPrimitive& c2)
	{
		return (c1.mask() & c2.layer()) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return collides(e1, e2);
				}, c2.get_baked());
			}, c1.get_baked()) : CollisionResult{ .overlap = false };
	}
	inline CollisionResult collides(const TPrimitive& c1, const Primitive& c2)
	{
		return (c1.mask() & c2.layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return collides(e1, e2);
				}, c2.element);
			}, c1.get_baked()) : CollisionResult{ .overlap = false };
	}
	inline CollisionResult collides(const Primitive& c1, const TPrimitive& c2)
	{
		return (c1.mask & c2.layer()) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return collides(e1, e2);
				}, c2.get_baked());
			}, c1.element) : CollisionResult{ .overlap = false };
	}
	inline ContactResult contacts(const TPrimitive& c1, const TPrimitive& c2)
	{
		return (c1.mask() & c2.layer()) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return contacts(e1, e2);
				}, c2.get_baked());
			}, c1.get_baked()) : ContactResult{ .overlap = false };
	}
	inline ContactResult contacts(const TPrimitive& c1, const Primitive& c2)
	{
		return (c1.mask() & c2.layer) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return contacts(e1, e2);
				}, c2.element);
			}, c1.get_baked()) : ContactResult{ .overlap = false };
	}
	inline ContactResult contacts(const Primitive& c1, const TPrimitive& c2)
	{
		return (c1.mask & c2.layer()) ? std::visit([c2](auto&& e1) {
			return std::visit([&e1](auto&& e2) {
				return contacts(e1, e2);
				}, c2.get_baked());
			}, c1.element) : ContactResult{ .overlap = false };
	}
}
