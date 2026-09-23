#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/elements/Circle.h"
#include "physics/collision/elements/AABB.h"
#include "physics/collision/elements/OBB.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/KDOP.h"
#include "physics/collision/Tolerance.h"

#include "core/containers/BlackBox.h"
#include "core/base/Parameters.h"
#include "core/types/Variant.h"

#include <imp/empty.hpp>
#include <imp/type_delimiter.hpp>

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

	class Element
	{
        imp::type_erasure _type = imp::erase_type<imp::empty>();
		BlackBox _obj;

	public:
		Element() = default;

		template<internal::Elem_check Shape>
		Element(Shape&& shape) : _obj(std::forward<Shape>(shape)) { _type = imp::erase_type<Shape>(); }

		template<internal::Elem_check Shape>
		Element& operator=(Shape&& shape)
		{
			if (imp::erase_type<Shape>() == _type)
				*_obj.cast<std::decay_t<Shape>>() = std::forward<Shape>(shape);
			else
			{
				_obj = BlackBox(std::forward<Shape>(shape));
				_type = imp::erase_type<Shape>();
			}
			return *this;
		}

		float projection_max(UnitVector2D axis) const;
		float projection_min(UnitVector2D axis) const;
		fpair projection_interval(UnitVector2D axis) const;
		ContactManifold deepest_manifold(UnitVector2D axis) const;
		Element transformed(const glm::mat3& m) const;

		using ConstElementVariant = Variant<
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

		using ElementVariant = Variant<
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
