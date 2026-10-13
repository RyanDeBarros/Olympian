#pragma once

#include "core/base/TransformerExposure.h"
#include "core/base/Transforms.h"

#include "physics/collision/Tolerance.h"

#include "physics/collision/elements/Element.h"

#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/Compound.h"
#include "physics/collision/methods/KDOPCollide.h"

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

		template<internal::Elem_check Shape>
		const Shape& element_as(size_t i) const
		{
			return *elements[i].variant().get<const Shape*>();
		}

		template<internal::Elem_check Shape>
		Shape& element_as(size_t i)
		{
			return *elements[i].variant().get<Shape*>();
		}
	};

	namespace internal { struct LUT; };

	class TCompound
	{
		friend struct internal::LUT;

		Transformer2D transformer_;
		Compound compound_;
		mutable std::vector<Element> baked_;
		mutable bool dirty_ = true;

	public:
		TCompound() = default;
        explicit TCompound(std::vector<Element> elements);

        const Transform2D& get_local() const;
        Transform2D& set_local();
        bool is_dirty() const;

        Transformer2DExposure<> get_transformer() const;
        Transformer2DExposure<ex::Full> set_transformer();

        const Compound& get_compound() const;
        Compound& set_compound();
        const std::vector<Element>& get_baked() const;

        Mask mask() const;
        Mask& mask();
        Layer layer() const;
        Layer& layer();

        const CompoundPerfParameters& perf() const;
        CompoundPerfParameters& perf();

		float projection_max(UnitVector2D axis) const;
		float projection_min(UnitVector2D axis) const;

		template<internal::Elem_check Shape>
		const Shape& element_as(size_t i) const
		{
			return get_compound().element_as<Shape>(i);
		}

		template<internal::Elem_check Shape>
		Shape& element_as(size_t i)
		{
			return set_compound().element_as<Shape>(i);
		}
        
    private:
        void bake() const;
	};

	namespace internal
	{
        inline const std::vector<Element>& element_array(const Compound& c);
        inline const std::vector<Element>& element_array(const TCompound& c);
        inline CompoundPerfParameters perf(const Compound& c);
        inline CompoundPerfParameters perf(const TCompound& c);

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
				return compound_collision(
                    element_array(c1).data(),
                    element_array(c1).size(),
                    element_array(c2).data(),
                    element_array(c2).size(),
                    CompoundPerfParameters::greedy(perf(c1), perf(c2)));
			else
				return CollisionResult{ .overlap = false };
		}

		template<typename Comp1, typename Comp2>
		inline ContactResult contacts(const Comp1& c1, const Comp2& c2)
		{
			if (overlaps(c1, c2))
				return compound_contact(
                    element_array(c1).data(),
                    element_array(c1).size(),
                    element_array(c2).data(),
                    element_array(c2).size(),
                    CompoundPerfParameters::greedy(perf(c1), perf(c2)));
			else
				return ContactResult{ .overlap = false };
		}

		template<typename Comp>
		OverlapResult overlaps(const Comp& c1, const Element& c2)
		{
			for (const auto& e1 : element_array(c1))
			{
				if (overlaps(e1, c2))
					return true;
			}
			return false;
		}

		template<typename Comp>
		CollisionResult collides(const Comp& c1, const Element& c2)
		{
			if (overlaps(c1, c2))
				return compound_collision(element_array(c1).data(), element_array(c1).size(), c2, perf(c1));
			else
				return CollisionResult{ .overlap = false };
		}

		template<typename Comp>
		ContactResult contacts(const Comp& c1, const Element& c2)
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

	extern OverlapResult overlaps(const Compound& c1, const Element& c2);
	extern OverlapResult overlaps(const Element& c1, const Compound& c2);
	extern CollisionResult collides(const Compound& c1, const Element& c2);
	extern CollisionResult collides(const Element& c1, const Compound& c2);
	extern ContactResult contacts(const Compound& c1, const Element& c2);
	extern ContactResult contacts(const Element& c1, const Compound& c2);

	extern OverlapResult point_hits(const TCompound& c, glm::vec2 test);
	extern OverlapResult ray_hits(const TCompound& c, Ray ray);
	extern RaycastResult raycast(const TCompound& c, Ray ray);

	extern OverlapResult overlaps(const TCompound& c1, const Element& c2);
	extern OverlapResult overlaps(const Element& c1, const TCompound& c2);
	extern CollisionResult collides(const TCompound& c1, const Element& c2);
	extern CollisionResult collides(const Element& c1, const TCompound& c2);
	extern ContactResult contacts(const TCompound& c1, const Element& c2);
	extern ContactResult contacts(const Element& c1, const TCompound& c2);

	extern OverlapResult overlaps(const Compound& c1, const Compound& c2);
	extern CollisionResult collides(const Compound& c1, const Compound& c2);
	extern ContactResult contacts(const Compound& c1, const Compound& c2);
	extern OverlapResult overlaps(const TCompound& c1, const TCompound& c2);
	extern CollisionResult collides(const TCompound& c1, const TCompound& c2);
	extern ContactResult contacts(const TCompound& c1, const TCompound& c2);
	extern OverlapResult overlaps(const TCompound& c1, const Compound& c2);
	extern CollisionResult collides(const TCompound& c1, const Compound& c2);
	extern ContactResult contacts(const TCompound& c1, const Compound& c2);
	extern OverlapResult overlaps(const Compound& c1, const TCompound& c2);
	extern CollisionResult collides(const Compound& c1, const TCompound& c2);
	extern ContactResult contacts(const Compound& c1, const TCompound& c2);
}
