#pragma once

#include "core/base/Parameters.h"

#include <set>

namespace oly::physics
{
	struct LinearCollisionDamping
	{
		// linear teleportation values under jitter threshold are fully dampened.
		PositiveFloat teleportation_jitter_threshold = 0.1f;
	};
	
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
		// snapping occurs when speed is under speed_threshold
		PositiveFloat speed_threshold = 0.1f;
		// snapping occurs when angle is within angle_threshold of the snapping rays
		PositiveFloat angle_threshold = glm::radians(10.0f);
		// defines the angles of the snapping rays
		std::set<BoundedRadians> snaps;
		// exponent on snapping proportion -> higher means faster snapping
		StrictlyPositiveFloat strength = 1.0f;
		// offset to snapping proportion after exponentiated:
		// At 0.0 - proportion has full effect on speed
		// At 1.0 - proportion has no effect on speed
		BoundedUnitInterval strength_offset = 0.2f;

		void set_uniformly_spaced_without_threshold(const size_t angles, float angle_offset = 0.0f);
		void set_uniformly_spaced(const size_t angles, float angle_offset = 0.0f);
	};

	struct LinearSnapping
	{
		// snapping occurs when speed is under speed_threshold
		PositiveFloat speed_threshold = 0.1f;
		// snapping occurs when position is within position_threshold of the snapping grid
		PositiveFloat position_threshold = 3.0f;
		// defines the width of cells in the snapping grid
		float snap_width = 50.0f;
		// defines the offset of the first cell in the snapping grid
		float snap_offset = 0.0f;
		// exponent on snapping proportion -> higher means faster snapping
		StrictlyPositiveFloat strength = 1.0f;
		// offset to snapping proportion after exponentiated:
		// At 0.0 - proportion has full effect on speed
		// At 1.0 - proportion has no effect on speed
		BoundedUnitInterval strength_offset = 0.2f;
	};
}
