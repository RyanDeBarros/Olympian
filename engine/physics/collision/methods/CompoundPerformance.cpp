#include "CompoundPerformance.h"

namespace oly::col2d
{
	void CompoundPerfParameters::set_coarse_sweep_divisions(size_t divisions)
	{
		coarse_sweep_divisions = glm::clamp(divisions, MIN_DIVISIONS, MAX_DIVISIONS);
		two_pi_over_divisions = glm::two_pi<float>() / coarse_sweep_divisions;
	}
	
	CompoundPerfParameters CompoundPerfParameters::greedy(const CompoundPerfParameters p1, const CompoundPerfParameters p2)
	{
		CompoundPerfParameters perf;
		perf.set_coarse_sweep_divisions(std::max(p1.get_coarse_sweep_divisions(), p2.get_coarse_sweep_divisions()));
		perf.refinement_error_threshold = std::min(p1.refinement_error_threshold, p2.refinement_error_threshold);
		return perf;
	}

	namespace perf
	{
		CompoundPerfParameters aabb(float width, float height, AABBParams params)
		{
			CompoundPerfParameters perf;
			if (width == 0.0f || height == 0.0f)
				return perf;
			float aspect = std::max(width, height) / std::min(width, height);
			if (aspect < params.thin_aspect)
				perf.set_coarse_sweep_divisions(16);
			perf.refinement_error_threshold = aspect < params.thin_aspect ? params.thick_refinement_threshold : params.thin_refinement_threshold;
			return perf;
		}

		CompoundPerfParameters circle(CircleParams params)
		{
			CompoundPerfParameters perf;
			perf.set_coarse_sweep_divisions(4);
			perf.refinement_error_threshold = params.refinement_threshold;
			return perf;
		}

		CompoundPerfParameters capsule(float obb_width, float obb_height, CapsuleParams params)
		{
			CompoundPerfParameters perf;
			if (obb_width == 0.0f || obb_height == 0.0f)
				return perf;
			float aspect = std::max(obb_width, obb_height) / std::min(obb_width, obb_height);
			if (aspect < params.obb_thin_aspect)
				perf.set_coarse_sweep_divisions(16);
			perf.refinement_error_threshold = aspect < params.obb_thin_aspect ? params.thick_refinement_threshold : params.thin_refinement_threshold;
			return perf;
		}
	}
}
