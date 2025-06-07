#pragma once

#include "physics/collision/elements/Element.h"
#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "physics/collision/Tolerance.h"
#include "core/base/Transforms.h"

namespace oly::col2d
{
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
		mutable bool dirty = true;

		void bake() const;

	public:
		Transformer2D transformer;

		TCompound() = default;
		TCompound(const std::vector<Element>& elements) : compound({ elements }) {}
		TCompound(std::vector<Element>&& elements) : compound({ std::move(elements) }) {}

		glm::mat3 global() const { return transformer.global(); }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { flag(); return transformer.set_local(); }

		void flag() const { dirty = true; }
		const Compound& get_compound() const { return compound; }
		Compound& set_compound() { flag(); return compound; }
		const std::vector<Element>& get_baked() const { if (dirty) { bake(); } return baked; }

		Mask mask() const { return compound.mask; }
		Mask& mask() { return compound.mask; }
		Layer layer() const { return compound.layer; }
		Layer& layer() { return compound.layer; }
	};

	extern OverlapResult point_hits(const TCompound& c, glm::vec2 test);
	extern OverlapResult ray_hits(const TCompound& c, const Ray& ray);
	extern RaycastResult raycast(const TCompound& c, const Ray& ray);
}
