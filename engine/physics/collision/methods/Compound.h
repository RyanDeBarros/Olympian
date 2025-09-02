#pragma once

#include "physics/collision/elements/Element.h"

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

	// TODO v4 utility functions for common CompoundPerfParameters for known shapes.

	extern CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements,
		const Element& static_element, const CompoundPerfParameters perf = {});
	extern CollisionResult compound_collision(const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const CompoundPerfParameters perf = {});
	extern ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const Element& static_element, const CompoundPerfParameters perf = {});
	extern ContactResult compound_contact(const Element* active_elements, const size_t num_active_elements,
		const Element* static_elements, const size_t num_static_elements, const CompoundPerfParameters perf = {});
}
