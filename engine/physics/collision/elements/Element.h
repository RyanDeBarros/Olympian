#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/elements/Circle.h"
#include "physics/collision/elements/AABB.h"
#include "physics/collision/elements/OBB.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/KDOP.h"
#include "physics/collision/Tolerance.h"

#include "core/base/Parameters.h"

#include <imp/box.hpp>
#include <imp/empty.hpp>
#include <imp/type_delimiter.hpp>
#include <imp/variant.hpp>

namespace oly::col2d
{
	using KDOP2 = KDOP<2>;
	using KDOP3 = KDOP<3>;
	using KDOP4 = KDOP<4>;
	using KDOP5 = KDOP<5>;
	using KDOP6 = KDOP<6>;
	using KDOP7 = KDOP<7>;
	using KDOP8 = KDOP<8>;

	namespace internal
	{
#define _OLY_ELEM_GENERATOR(M) \
        M((imp::empty)) \
        M((Circle)) \
        M((AABB)) \
        M((OBB)) \
        M((ConvexHull)) \
        M((KDOP2)) \
        M((KDOP3)) \
        M((KDOP4)) \
        M((KDOP5)) \
        M((KDOP6)) \
        M((KDOP7)) \
        M((KDOP8))

        IMP_TYPE_DELIMITER(_OLY_ELEM_GENERATOR, Elem);
	}

    // TODO v9.3 imp::box addition seems to mess up physics in Tester, perhaps because of imp::empty

	class Element
	{
		imp::box _obj;

	public:
        Element() : _obj(imp::make_box<imp::empty>()) {}

		template<internal::Elem_check Shape>
		Element(Shape&& shape) : _obj(imp::forward_to_box(std::forward<Shape>(shape))) {}

		template<internal::Elem_check Shape>
		Element& operator=(Shape&& shape)
		{
            imp::forward_into_box(_obj, std::forward<Shape>(shape));
			return *this;
		}

		float projection_max(UnitVector2D axis) const;
		float projection_min(UnitVector2D axis) const;
		fpair projection_interval(UnitVector2D axis) const;
		ContactManifold deepest_manifold(UnitVector2D axis) const;
		Element transformed(const glm::mat3& m) const;

		using ConstElementVariant = imp::variant<
			const Circle*,
			const AABB*,
			const OBB*,
			const ConvexHull*,
			const KDOP2*,
			const KDOP3*,
			const KDOP4*,
			const KDOP5*,
			const KDOP6*,
			const KDOP7*,
			const KDOP8*
		>;

		using ElementVariant = imp::variant<
			Circle*,
			AABB*,
			OBB*,
			ConvexHull*,
			KDOP2*,
			KDOP3*,
			KDOP4*,
			KDOP5*,
			KDOP6*,
			KDOP7*,
			KDOP8*
		>;

		ConstElementVariant variant() const;
		ElementVariant variant();
		AABB aabb_wrap() const;

		OverlapResult point_hits(glm::vec2 test) const;
		OverlapResult ray_hits(Ray ray) const;
		RaycastResult raycast(Ray ray) const;
		OverlapResult overlaps(const Element& c) const;
		CollisionResult collides(const Element& c) const;
		ContactResult contacts(const Element& c) const;
	};

	typedef unsigned int Mask;
	typedef unsigned int Layer;
}
