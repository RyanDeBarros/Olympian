#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/elements/Circle.h"
#include "physics/collision/elements/AABB.h"
#include "physics/collision/elements/OBB.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/KDOP.h"
#include "physics/collision/Tolerance.h"

#include "core/containers/CopyPtr.h"
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

	// TODO v2 black box with id instead of variant
	using Element = std::variant<
		Circle,
		AABB,
		OBB,
		ConvexHull,
		CopyPtr<KDOP2>,
		CopyPtr<KDOP3>,
		CopyPtr<KDOP4>,
		CopyPtr<KDOP5>,
		CopyPtr<KDOP6>,
		CopyPtr<KDOP7>,
		CopyPtr<KDOP8>
	>;

	inline Element element(const Circle& c) { return c; }
	inline Element element(const AABB& c) { return c; }
	inline Element element(const OBB& c) { return c; }
	inline Element element(const ConvexHull& c) { return c; }
	inline Element element(ConvexHull&& c) { return std::move(c); }
	inline Element element(const KDOP2& c) { return CopyPtr<KDOP2>(c); }
	inline Element element(KDOP2&& c) { return CopyPtr<KDOP2>(std::move(c)); }
	inline Element element(const KDOP3& c) { return CopyPtr<KDOP3>(c); }
	inline Element element(KDOP3&& c) { return CopyPtr<KDOP3>(std::move(c)); }
	inline Element element(const KDOP4& c) { return CopyPtr<KDOP4>(c); }
	inline Element element(KDOP4&& c) { return CopyPtr<KDOP4>(std::move(c)); }
	inline Element element(const KDOP5& c) { return CopyPtr<KDOP5>(c); }
	inline Element element(KDOP5&& c) { return CopyPtr<KDOP5>(std::move(c)); }
	inline Element element(const KDOP6& c) { return CopyPtr<KDOP6>(c); }
	inline Element element(KDOP6&& c) { return CopyPtr<KDOP6>(std::move(c)); }
	inline Element element(const KDOP7& c) { return CopyPtr<KDOP7>(c); }
	inline Element element(KDOP7&& c) { return CopyPtr<KDOP7>(std::move(c)); }
	inline Element element(const KDOP8& c) { return CopyPtr<KDOP8>(c); }
	inline Element element(KDOP8&& c) { return CopyPtr<KDOP8>(std::move(c)); }

	struct ElementPtr
	{
	private:
		enum class ID
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
		} id = ID::NONE;

		const void* ptr = nullptr;

	public:
		ElementPtr() = default;
		explicit ElementPtr(const Element& e)    { set(e); }
		explicit ElementPtr(const Circle& c)     { set(c); }
		explicit ElementPtr(const AABB& c)       { set(c); }
		explicit ElementPtr(const OBB& c)        { set(c); }
		explicit ElementPtr(const ConvexHull& c) { set(c); }
		explicit ElementPtr(const KDOP2& c)      { set(c); }
		explicit ElementPtr(const KDOP3& c)      { set(c); }
		explicit ElementPtr(const KDOP4& c)      { set(c); }
		explicit ElementPtr(const KDOP5& c)      { set(c); }
		explicit ElementPtr(const KDOP6& c)      { set(c); }
		explicit ElementPtr(const KDOP7& c)      { set(c); }
		explicit ElementPtr(const KDOP8& c)      { set(c); }

		void set(const Element& e);
		void set(const Circle& c)     { ptr = &c; id = ID::CIRCLE; }
		void set(const AABB& c)       { ptr = &c; id = ID::AABB; }
		void set(const OBB& c)        { ptr = &c; id = ID::OBB; }
		void set(const ConvexHull& c) { ptr = &c; id = ID::CONVEX_HULL; }
		void set(const KDOP2& c)      { ptr = &c; id = ID::KDOP2; }
		void set(const KDOP3& c)      { ptr = &c; id = ID::KDOP3; }
		void set(const KDOP4& c)      { ptr = &c; id = ID::KDOP4; }
		void set(const KDOP5& c)      { ptr = &c; id = ID::KDOP5; }
		void set(const KDOP6& c)      { ptr = &c; id = ID::KDOP6; }
		void set(const KDOP7& c)      { ptr = &c; id = ID::KDOP7; }
		void set(const KDOP8& c)      { ptr = &c; id = ID::KDOP8; }

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
		OverlapResult overlaps(ElementPtr c) const;
		CollisionResult collides(ElementPtr c) const;
		ContactResult contacts(ElementPtr c) const;
	};

	namespace internal
	{
		extern Element transform_element(const Circle& c, const glm::mat3& m);
		extern Element transform_element(const AABB& c, const glm::mat3& m);
		extern Element transform_element(const OBB& c, const glm::mat3& m);
		extern Element transform_element(const KDOP2& c, const glm::mat3& m);
		extern Element transform_element(const KDOP3& c, const glm::mat3& m);
		extern Element transform_element(const KDOP4& c, const glm::mat3& m);
		extern Element transform_element(const KDOP5& c, const glm::mat3& m);
		extern Element transform_element(const KDOP6& c, const glm::mat3& m);
		extern Element transform_element(const KDOP7& c, const glm::mat3& m);
		extern Element transform_element(const KDOP8& c, const glm::mat3& m);
		extern Element transform_element(const ConvexHull& c, const glm::mat3& m);
	}

	typedef int Mask;
	typedef int Layer;
}
