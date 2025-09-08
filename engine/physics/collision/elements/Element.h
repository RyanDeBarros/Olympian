#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/elements/Circle.h"
#include "physics/collision/elements/AABB.h"
#include "physics/collision/elements/OBB.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/KDOP.h"
#include "physics/collision/Tolerance.h"

#include "core/containers/CopyPtr.h"
#include "core/containers/BlackBox.h"
#include "core/base/Parameters.h"

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
		enum class ElementID
		{
			NONE,
			CIRCLE,
			AABB,
			OBB,
			CONVEX_HULL,
			KDOP2,
			KDOP3,
			KDOP4,
			KDOP5,
			KDOP6,
			KDOP7,
			KDOP8
		};

		template<typename T>
		struct ElementIDTrait;

		template<typename T>
		concept ElementShape = requires { { ElementIDTrait<std::decay_t<T>>::ID } -> std::convertible_to<ElementID>; };

		template<>
		struct ElementIDTrait<Circle>
		{
			static constexpr ElementID ID = ElementID::CIRCLE;
		};

		template<>
		struct ElementIDTrait<AABB>
		{
			static constexpr ElementID ID = ElementID::AABB;
		};

		template<>
		struct ElementIDTrait<OBB>
		{
			static constexpr ElementID ID = ElementID::OBB;
		};

		template<>
		struct ElementIDTrait<ConvexHull>
		{
			static constexpr ElementID ID = ElementID::CONVEX_HULL;
		};

		template<>
		struct ElementIDTrait<KDOP2>
		{
			static constexpr ElementID ID = ElementID::KDOP2;
		};

		template<>
		struct ElementIDTrait<KDOP3>
		{
			static constexpr ElementID ID = ElementID::KDOP3;
		};

		template<>
		struct ElementIDTrait<KDOP4>
		{
			static constexpr ElementID ID = ElementID::KDOP4;
		};

		template<>
		struct ElementIDTrait<KDOP5>
		{
			static constexpr ElementID ID = ElementID::KDOP5;
		};

		template<>
		struct ElementIDTrait<KDOP6>
		{
			static constexpr ElementID ID = ElementID::KDOP6;
		};

		template<>
		struct ElementIDTrait<KDOP7>
		{
			static constexpr ElementID ID = ElementID::KDOP7;
		};

		template<>
		struct ElementIDTrait<KDOP8>
		{
			static constexpr ElementID ID = ElementID::KDOP8;
		};
	}

	class Element
	{
		internal::ElementID id = internal::ElementID::NONE;
		BlackBox<true> obj;

	public:
		Element() = default;
		template<internal::ElementShape Shape>
		Element(Shape&& shape) : obj(std::forward<Shape>(shape)) { id = internal::ElementIDTrait<std::decay_t<Shape>>::ID; }

		template<internal::ElementShape Shape>
		Element& operator=(Shape&& shape)
		{
			if (internal::ElementIDTrait<std::decay_t<Shape>>::ID == id)
				*obj.cast<std::decay_t<Shape>>() = std::forward<Shape>(shape);
			else
			{
				obj = BlackBox<true>(std::forward<Shape>(shape));
				id = internal::ElementIDTrait<std::decay_t<Shape>>::ID;
			}
			return *this;
		}

		float projection_max(UnitVector2D axis) const;
		float projection_min(UnitVector2D axis) const;
		fpair projection_interval(UnitVector2D axis) const;
		ContactManifold deepest_manifold(UnitVector2D axis) const;
		Element transformed(const glm::mat3& m) const;

		using ElementVariant = std::variant<
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

		ElementVariant variant() const;
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
