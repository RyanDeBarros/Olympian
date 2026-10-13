#pragma once

#include "core/base/Parameters.h"

namespace oly::col2d
{
	class CompoundPerfParameters
	{
		size_t coarse_sweep_divisions_ = 24;
		float two_pi_over_divisions_ = glm::two_pi<float>() / coarse_sweep_divisions_;

	public:
		StrictlyPositiveFloat refinement_error_threshold = glm::radians(1.5f);

        static CompoundPerfParameters greedy(const CompoundPerfParameters p1, const CompoundPerfParameters p2);

		void set_coarse_sweep_divisions(size_t divisions);
        size_t get_coarse_sweep_divisions() const;
        float get_two_pi_over_divisions() const;
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
