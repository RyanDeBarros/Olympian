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

	// TODO v2 wrap const void* and use id + static_cast instead of variant
	using ElementParam = std::variant<
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

	extern ElementParam param(const Element& e);
	inline ElementParam param(const Circle& c) { return &c; }
	inline ElementParam param(const AABB& c) { return &c; }
	inline ElementParam param(const OBB& c) { return &c; }
	inline ElementParam param(const ConvexHull& c) { return &c; }
	inline ElementParam param(const KDOP2& c) { return &c; }
	inline ElementParam param(const KDOP3& c) { return &c; }
	inline ElementParam param(const KDOP4& c) { return &c; }
	inline ElementParam param(const KDOP5& c) { return &c; }
	inline ElementParam param(const KDOP6& c) { return &c; }
	inline ElementParam param(const KDOP7& c) { return &c; }
	inline ElementParam param(const KDOP8& c) { return &c; }

	class CompoundPerfParameters
	{
		static const size_t MIN_DIVISIONS = 16;
		static const size_t MAX_DIVISIONS = 32;
		size_t coarse_sweep_divisions = 24;
		float two_pi_over_divisions = glm::two_pi<float>() / coarse_sweep_divisions;

	public:
		StrictlyPositiveFloat refinement_error_threshold = glm::radians(1.5f);

		void set_coarse_sweep_divisions(size_t divisions)
		{
			coarse_sweep_divisions = glm::clamp(divisions, MIN_DIVISIONS, MAX_DIVISIONS);
			two_pi_over_divisions = glm::two_pi<float>() / coarse_sweep_divisions;
		}

		size_t get_coarse_sweep_divisions() const { return coarse_sweep_divisions; }

		float get_two_pi_over_divisions() const { return two_pi_over_divisions; }

		static CompoundPerfParameters greedy(const CompoundPerfParameters p1, const CompoundPerfParameters p2)
		{
			CompoundPerfParameters pg;
			pg.set_coarse_sweep_divisions(std::max(p1.get_coarse_sweep_divisions(), p2.get_coarse_sweep_divisions()));
			pg.refinement_error_threshold = std::max(p1.refinement_error_threshold, p2.refinement_error_threshold);
			return pg;
		}
	};

	// LATER utility functions for common CompoundPerfParameters for known shapes.

	extern CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements,
		const ElementParam& static_element, const CompoundPerfParameters perf = {});
	extern CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const CompoundPerfParameters perf = {});
	extern ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const ElementParam& static_element, const CompoundPerfParameters perf = {});
	extern ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const CompoundPerfParameters perf = {});

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

	inline Element transform_element(const ElementParam& c, const glm::mat3& m) { return std::visit([&m](auto&& c) { return internal::transform_element(*c, m); }, c); }

	typedef int Mask;
	typedef int Layer;
}
