#pragma once

#include "physics/collision/elements/Element.h"
#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "physics/collision/methods/Compound.h"
#include "physics/collision/Tolerance.h"
#include "core/base/Transforms.h"
#include "core/base/TransformerExposure.h"

namespace oly::col2d
{
	struct Compound
	{
		std::vector<Element> elements;

		Mask mask = 0;
		Layer layer = 0;

		CompoundPerfParameters perf = {};

		float projection_max(UnitVector2D axis) const;
		float projection_min(UnitVector2D axis) const;
	};

	namespace internal { struct LUT; };

	class TCompound
	{
		friend struct internal::LUT;

		Transformer2D transformer;
		Compound compound;
		mutable std::vector<Element> baked;
		mutable bool dirty = true;

		void bake() const;

	public:
		TCompound() = default;
		explicit TCompound(const std::vector<Element>& elements) : compound({ elements }) {}
		explicit TCompound(std::vector<Element>&& elements) : compound({ std::move(elements) }) {}

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }
		bool is_dirty() const { return dirty || transformer.dirty(); }

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<TExposureParams{ .local = exposure::local::FULL, .chain = exposure::chain::FULL, .modifier = exposure::modifier::FULL }> set_transformer() { return transformer; }

		const Compound& get_compound() const { return compound; }
		Compound& set_compound() { dirty = true; return compound; }
		const std::vector<Element>& get_baked() const { if (dirty || transformer.flush()) { bake(); } return baked; }

		Mask mask() const { return compound.mask; }
		Mask& mask() { return compound.mask; }
		Layer layer() const { return compound.layer; }
		Layer& layer() { return compound.layer; }

		const CompoundPerfParameters& perf() const { return compound.perf; }
		CompoundPerfParameters& perf() { return compound.perf; }

		float projection_max(UnitVector2D axis) const;
		float projection_min(UnitVector2D axis) const;
	};

	namespace internal
	{
		inline const std::vector<Element>& element_array(const Compound& c) { return c.elements; }
		inline const std::vector<Element>& element_array(const TCompound& c) { return c.get_baked(); }
		inline CompoundPerfParameters perf(const Compound& c) { return c.perf; }
		inline CompoundPerfParameters perf(const TCompound& c) { return c.perf(); }

		template<typename Comp1, typename Comp2>
		inline OverlapResult overlaps(const Comp1& c1, const Comp2& c2)
		{
			for (const auto& e1 : element_array(c1))
				for (const auto& e2 : element_array(c2))
					if (overlaps(e1, e2))
						return true;
			return false;
		}

		template<typename Comp1, typename Comp2>
		inline CollisionResult collides(const Comp1& c1, const Comp2& c2)
		{
			if (overlaps(c1, c2))
				return compound_collision(element_array(c1).data(), element_array(c1).size(), element_array(c2).data(), element_array(c2).size(), CompoundPerfParameters::greedy(perf(c1), perf(c2)));
			else
				return CollisionResult{ .overlap = false };
		}

		template<typename Comp1, typename Comp2>
		inline ContactResult contacts(const Comp1& c1, const Comp2& c2)
		{
			if (overlaps(c1, c2))
				return compound_contact(element_array(c1).data(), element_array(c1).size(), element_array(c2).data(), element_array(c2).size(), CompoundPerfParameters::greedy(perf(c1), perf(c2)));
			else
				return ContactResult{ .overlap = false };
		}

		template<typename Comp>
		OverlapResult overlaps(const Comp& c1, ElementPtr c2)
		{
			for (const auto& e1 : element_array(c1))
			{
				if (overlaps(ElementPtr(e1), c2))
					return true;
			}
			return false;
		}

		template<typename Comp>
		CollisionResult collides(const Comp& c1, ElementPtr c2)
		{
			if (overlaps(c1, c2))
				return compound_collision(element_array(c1).data(), element_array(c1).size(), c2, perf(c1));
			else
				return CollisionResult{ .overlap = false };
		}

		template<typename Comp>
		ContactResult contacts(const Comp& c1, ElementPtr c2)
		{
			if (overlaps(c1, c2))
				return compound_contact(element_array(c1).data(), element_array(c1).size(), c2, perf(c1));
			else
				return ContactResult{ .overlap = false };
		}
	}

	extern OverlapResult point_hits(const Compound& c, glm::vec2 test);
	extern OverlapResult ray_hits(const Compound& c, Ray ray);
	extern RaycastResult raycast(const Compound& c, Ray ray);

	inline OverlapResult overlaps(const Compound& c1, ElementPtr c2) { return internal::overlaps(c1, c2); }
	inline OverlapResult overlaps(ElementPtr c1, const Compound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const Compound& c1, ElementPtr c2) { return internal::collides(c1, c2); }
	inline CollisionResult collides(ElementPtr c1, const Compound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const Compound& c1, ElementPtr c2) { return internal::contacts(c1, c2); }
	inline ContactResult contacts(ElementPtr c1, const Compound& c2) { return contacts(c2, c1).invert(); }

	extern OverlapResult point_hits(const TCompound& c, glm::vec2 test);
	extern OverlapResult ray_hits(const TCompound& c, Ray ray);
	extern RaycastResult raycast(const TCompound& c, Ray ray);

	inline OverlapResult overlaps(const TCompound& c1, ElementPtr c2) { return internal::overlaps(c1, c2); }
	inline OverlapResult overlaps(ElementPtr c1, const TCompound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const TCompound& c1, ElementPtr c2) { return internal::collides(c1, c2); }
	inline CollisionResult collides(ElementPtr c1, const TCompound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const TCompound& c1, ElementPtr c2) { return internal::contacts(c1, c2); }
	inline ContactResult contacts(ElementPtr c1, const TCompound& c2) { return contacts(c2, c1).invert(); }

	inline OverlapResult overlaps(const Compound& c1, const Compound& c2) { return (c1.mask & c2.layer) && internal::overlaps(c1, c2); }
	inline CollisionResult collides(const Compound& c1, const Compound& c2) { return (c1.mask & c2.layer) ? internal::collides(c1, c2) : CollisionResult{ .overlap = false }; }
	inline ContactResult contacts(const Compound& c1, const Compound& c2) { return (c1.mask & c2.layer) ? internal::contacts(c1, c2) : ContactResult{ .overlap = false }; }
	inline OverlapResult overlaps(const TCompound& c1, const TCompound& c2) { return (c1.mask() & c2.layer()) && internal::overlaps(c1, c2); }
	inline CollisionResult collides(const TCompound& c1, const TCompound& c2) { return (c1.mask() & c2.layer()) ? internal::collides(c1, c2) : CollisionResult{ .overlap = false }; }
	inline ContactResult contacts(const TCompound& c1, const TCompound& c2) { return (c1.mask() & c2.layer()) ? internal::contacts(c1, c2) : ContactResult{ .overlap = false }; }
	inline OverlapResult overlaps(const TCompound& c1, const Compound& c2) { return (c1.mask() & c2.layer) && internal::overlaps(c1, c2); }
	inline CollisionResult collides(const TCompound& c1, const Compound& c2) { return (c1.mask() & c2.layer) ? internal::collides(c1, c2) : CollisionResult{ .overlap = false }; }
	inline ContactResult contacts(const TCompound& c1, const Compound& c2) { return (c1.mask() & c2.layer) ? internal::contacts(c1, c2) : ContactResult{ .overlap = false }; }
	inline OverlapResult overlaps(const Compound& c1, const TCompound& c2) { return (c1.mask & c2.layer()) && internal::overlaps(c1, c2); }
	inline CollisionResult collides(const Compound& c1, const TCompound& c2) { return (c1.mask & c2.layer()) ? internal::collides(c1, c2) : CollisionResult{ .overlap = false }; }
	inline ContactResult contacts(const Compound& c1, const TCompound& c2) { return (c1.mask & c2.layer()) ? internal::contacts(c1, c2) : ContactResult{ .overlap = false }; }
}
