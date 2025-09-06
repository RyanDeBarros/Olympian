#pragma once

#include "core/base/Parameters.h"

namespace oly::col2d
{
	class CompoundPerfParameters
	{
		static const size_t MIN_DIVISIONS = 16;
		static const size_t MAX_DIVISIONS = 32;
		size_t coarse_sweep_divisions = 24;
		float two_pi_over_divisions = glm::two_pi<float>() / coarse_sweep_divisions;

	public:
		StrictlyPositiveFloat refinement_error_threshold = glm::radians(1.5f);

		void set_coarse_sweep_divisions(size_t divisions);
		size_t get_coarse_sweep_divisions() const { return coarse_sweep_divisions; }
		float get_two_pi_over_divisions() const { return two_pi_over_divisions; }

		static CompoundPerfParameters greedy(const CompoundPerfParameters p1, const CompoundPerfParameters p2);
	};

	namespace perf
	{
		struct AABBParams
		{
			float thin_aspect = 2.5f;
			StrictlyPositiveFloat thick_refinement_threshold = glm::radians(2.0f);
			StrictlyPositiveFloat thin_refinement_threshold = glm::radians(0.5f);
		};
		extern CompoundPerfParameters aabb(float width, float height, AABBParams params = {});

		struct CircleParams
		{
			StrictlyPositiveFloat refinement_threshold = glm::radians(2.0f);
		};
		extern CompoundPerfParameters circle(CircleParams params = {});

		struct CapsuleParams
		{
			float obb_thin_aspect = 1.5f;
			StrictlyPositiveFloat thick_refinement_threshold = glm::radians(2.0f);
			StrictlyPositiveFloat thin_refinement_threshold = glm::radians(0.5f);
		};
		extern CompoundPerfParameters capsule(float obb_width, float obb_height, CapsuleParams params = {});
	}
}
