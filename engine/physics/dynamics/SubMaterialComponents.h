#pragma once

#include "core/base/Parameters.h"

#include <set>

namespace oly::physics
{
	struct AngularCollisionDamping
	{
		// angular teleportation damping controls how much corrective angular impulse is accumulated from collisions.
		// At 0.0 - no damping, which causes large angular bounces from collisions.
		// At 1.0 - full damping, which prevents any angular response to collisions.
		PowerInterval teleportation = PowerInterval(0.85f);

		// angular teleport inverse drag controls how higher teleportation levels are shrunken. A smaller inverse drag results in more overall teleportation damping.
		BoundedUnitInterval teleportation_inverse_drag = 0.6f;

		// angular teleportation values under jitter threshold are fully dampened.
		PositiveFloat teleportation_jitter_threshold = 0.005f;

		// Same parameters but for angular impulse due to restitution, not teleportation.
		PowerInterval restitution = PowerInterval(0.5f);
		BoundedUnitInterval restitution_inverse_drag = 0.9f;
		PositiveFloat restitution_jitter_threshold = 0.001f;
	};

	struct AngularSnapping
	{
		PositiveFloat speed_threshold = 0.1f;
		PositiveFloat angle_threshold = glm::radians(10.0f);
		std::set<BoundedRadians> snaps;
		StrictlyPositiveFloat strength = 1.0f;
		BoundedUnitInterval strength_offset = 0.2f;

		void set_uniformly_spaced_without_threshold(const size_t angles, float angle_offset = 0.0f);
		void set_uniformly_spaced(const size_t angles, float angle_offset = 0.0f);
	};

	struct LinearSnapping
	{
		PositiveFloat speed_threshold = 0.1f;
		PositiveFloat position_threshold = 3.0f;
		float snap_width = 50.0f;
		float snap_offset = 0.0f;
		StrictlyPositiveFloat strength = 1.0f;
		BoundedUnitInterval strength_offset = 0.2f;
	};
}
