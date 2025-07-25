#pragma once

#include "physics/collision/elements/Element.h"
#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "physics/collision/Tolerance.h"
#include "core/base/Transforms.h"
#include "core/base/TransformerExposure.h"

namespace oly::col2d
{
	struct Primitive
	{
		Element element;

		Mask mask = 0;
		Layer layer = 0;

		float projection_max(UnitVector2D axis) const { return ElementPtr(element).projection_max(axis); }
		float projection_min(UnitVector2D axis) const { return ElementPtr(element).projection_min(axis); }
		fpair projection_interval(UnitVector2D axis) const { return ElementPtr(element).projection_interval(axis); }
		ContactManifold deepest_manifold(UnitVector2D axis) const { return ElementPtr(element).deepest_manifold(axis); }
	};

	inline OverlapResult point_hits(const Primitive& c, glm::vec2 test) { return point_hits(c.element, test); }
	inline OverlapResult ray_hits(const Primitive& c, Ray ray) { return ray_hits(c.element, ray); }
	inline RaycastResult raycast(const Primitive& c, Ray ray) { return raycast(c.element, ray); }

#define OLY_ELEMENTS_OVERLAP 

	inline OverlapResult overlaps(const Primitive& c1, const Primitive& c2) { return (c1.mask & c2.layer) && overlaps(c1.element, c2.element); }
	inline CollisionResult collides(const Primitive& c1, const Primitive& c2) { return (c1.mask & c2.layer) ? collides(c1.element, c2.element) : CollisionResult{ .overlap = false }; }
	inline ContactResult contacts(const Primitive& c1, const Primitive& c2) { return (c1.mask & c2.layer) ? contacts(c1.element, c2.element) : ContactResult{ .overlap = false }; }

	namespace internal { struct LUT; };

	class TPrimitive
	{
		friend struct internal::LUT;
		Transformer2D transformer;
		Primitive primitive;
		mutable Element baked;
		mutable bool dirty = true;

		void bake() const
		{
			glm::mat3 g = transformer.global();
			baked = std::visit([&g](auto&& e) { return ElementPtr(e).transformed(g); }, primitive.element);
			dirty = false;
		}

	public:
		TPrimitive() = default;
		explicit TPrimitive(const Element& element) : primitive{ .element = element } {}
		explicit TPrimitive(Element&& element) noexcept : primitive{ .element = std::move(element) } {}

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }
		bool is_dirty() const { return dirty || transformer.dirty(); }

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<TExposureParams{ .local = exposure::local::FULL, .chain = exposure::chain::FULL, .modifier = exposure::modifier::FULL }> set_transformer() { return transformer; }

		const Primitive& get_primitive() const { return primitive; }
		Primitive& set_primitive() { dirty = true; return primitive; }
		const Element& get_baked() const { if (dirty || transformer.flush()) { bake(); } return baked; }

		Mask mask() const { return primitive.mask; }
		Mask& mask() { return primitive.mask; }
		Layer layer() const { return primitive.layer; }
		Layer& layer() { return primitive.layer; }

		float projection_max(UnitVector2D axis) const { return ElementPtr(get_baked()).projection_max(axis); }
		float projection_min(UnitVector2D axis) const { return ElementPtr(get_baked()).projection_min(axis); }
		fpair projection_interval(UnitVector2D axis) const { return ElementPtr(get_baked()).projection_interval(axis); }
		ContactManifold deepest_manifold(UnitVector2D axis) const { return ElementPtr(get_baked()).deepest_manifold(axis); }
	};

	inline OverlapResult point_hits(const TPrimitive& c, glm::vec2 test) { return point_hits(c.get_baked(), test); }
	inline OverlapResult ray_hits(const TPrimitive& c, Ray ray) { return ray_hits(c.get_baked(), ray); }
	inline RaycastResult raycast(const TPrimitive& c, Ray ray) { return raycast(c.get_baked(), ray); }

	inline OverlapResult overlaps(const TPrimitive& c1, const TPrimitive& c2) { return (c1.mask() & c2.layer()) && overlaps(c1.get_baked(), c2.get_baked()); }
	inline OverlapResult overlaps(const TPrimitive& c1, const Primitive& c2) { return (c1.mask() & c2.layer) && overlaps(c1.get_baked(), c2.element); }
	inline OverlapResult overlaps(const Primitive& c1, const TPrimitive& c2) { return (c1.mask & c2.layer()) && overlaps(c1.element, c2.get_baked()); }
	inline CollisionResult collides(const TPrimitive& c1, const TPrimitive& c2) { return (c1.mask() & c2.layer()) ? collides(c1.get_baked(), c2.get_baked()) : CollisionResult{ .overlap = false }; }
	inline CollisionResult collides(const TPrimitive& c1, const Primitive& c2) { return (c1.mask() & c2.layer) ? collides(c1.get_baked(), c2.element) : CollisionResult{ .overlap = false }; }
	inline CollisionResult collides(const Primitive& c1, const TPrimitive& c2) { return (c1.mask & c2.layer()) ? collides(c1.element, c2.get_baked()) : CollisionResult{ .overlap = false }; }
	inline ContactResult contacts(const TPrimitive& c1, const TPrimitive& c2) { return (c1.mask() & c2.layer()) ? contacts(c1.get_baked(), c2.get_baked()) : ContactResult{ .overlap = false }; }
	inline ContactResult contacts(const TPrimitive& c1, const Primitive& c2) { return (c1.mask() & c2.layer) ? contacts(c1.get_baked(), c2.element) : ContactResult{ .overlap = false }; }
	inline ContactResult contacts(const Primitive& c1, const TPrimitive& c2) { return (c1.mask & c2.layer()) ? contacts(c1.element, c2.get_baked()) : ContactResult{ .overlap = false }; }

	inline OverlapResult overlaps(const Primitive& c1, ElementPtr c2) { return overlaps(ElementPtr(c1.element), c2); }
	inline OverlapResult overlaps(ElementPtr c1, const Primitive& c2) { return overlaps(c1, ElementPtr(c2.element)); }
	inline CollisionResult collides(const Primitive& c1, ElementPtr c2) { return collides(ElementPtr(c1.element), c2); }
	inline CollisionResult collides(ElementPtr c1, const Primitive& c2) { return collides(c1, ElementPtr(c2.element)); }
	inline ContactResult contacts(const Primitive& c1, ElementPtr c2) { return contacts(ElementPtr(c1.element), c2); }
	inline ContactResult contacts(ElementPtr c1, const Primitive& c2) { return contacts(c1, ElementPtr(c2.element)); }

	inline OverlapResult overlaps(const TPrimitive& c1, ElementPtr c2) { return overlaps(ElementPtr(c1.get_baked()), c2); }
	inline OverlapResult overlaps(ElementPtr c1, const TPrimitive& c2) { return overlaps(c1, ElementPtr(c2.get_baked())); }
	inline CollisionResult collides(const TPrimitive& c1, ElementPtr c2) { return collides(ElementPtr(c1.get_baked()), c2); }
	inline CollisionResult collides(ElementPtr c1, const TPrimitive& c2) { return collides(c1, ElementPtr(c2.get_baked())); }
	inline ContactResult contacts(const TPrimitive& c1, ElementPtr c2) { return contacts(ElementPtr(c1.get_baked()), c2); }
	inline ContactResult contacts(ElementPtr c1, const TPrimitive& c2) { return contacts(c1, ElementPtr(c2.get_baked())); }
}
