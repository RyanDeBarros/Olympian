#pragma once

#include "core/base/Parameters.h"
#include "core/types/SmartHandle.h"

namespace oly::physics
{
	enum class FrictionType
	{
		STATIC,
		KINETIC,
		ROLLING
	};

	enum class FactorBlendOp
	{
		MINIMUM,
		GEOMETRIC_MEAN,
		ARITHMETIC_MEAN,
		ACTIVE
	};

	// LATER density
	struct Material
	{
	private:
		PositiveFloat _static_friction = 0.5f;
		PositiveFloat _sqrt_static_friction = glm::sqrt(_static_friction);
		PositiveFloat _kinetic_friction = 0.3f;
		PositiveFloat _sqrt_kinetic_friction = glm::sqrt(_kinetic_friction);
		PositiveFloat _rolling_friction = 0.4f;
		PositiveFloat _sqrt_rolling_friction = glm::sqrt(_rolling_friction);
		BoundedUnitInterval _restitution = 0.2f;
		BoundedUnitInterval _sqrt_restitution = glm::sqrt(_restitution);

	public:
		PositiveFloat linear_drag = 0.0f;
		PositiveFloat angular_drag = 1.0f;

		struct CollisionDamping
		{
			// linear penetration damping controls how much penetration motion into another collider is clamped.
			// At 0.0 - no clamping, which may cause clipping and significant friction due to high normal force.
			// At 1.0 - complete clamping, which may cause slight jitteriness or alternating colliding states.
			BoundedUnitInterval linear_penetration = 0.5f;
			
			// angular teleportation damping controls how much corrective angular impulse is accumulated from collisions.
			// At 0.0 - no damping, which causes large angular bounces from collisions.
			// At 1.0 - full damping, which prevents any angular response to collisions.
			PowerInterval angular_teleportation = PowerInterval(0.85f);
			
			// angular teleport inverse drag controls how higher teleportation levels are shrunken. A smaller inverse drag results in more overall teleportation damping.
			BoundedUnitInterval angular_teleport_inverse_drag = 0.6f;
			
			// angular teleportation values under jitter threshold are fully dampened.
			PositiveFloat angular_jitter_threshold = 0.005f;

			// Same parameters but for angular impulse due to restitution, not teleportation.
			PowerInterval angular_restitution = PowerInterval(0.5f);
			BoundedUnitInterval angular_bounce_inverse_drag = 0.9f;
			PositiveFloat angular_bounce_jitter_threshold = 0.001f;
		} collision_damping;

		struct Blending
		{
			FactorBlendOp restitution = FactorBlendOp::MINIMUM;
			FactorBlendOp static_friction = FactorBlendOp::GEOMETRIC_MEAN;
			FactorBlendOp kinetic_friction = FactorBlendOp::GEOMETRIC_MEAN;
			FactorBlendOp rolling_friction = FactorBlendOp::GEOMETRIC_MEAN;
		} blending;

		struct AngularSnapping
		{
			PositiveFloat speed_threshold = 0.1f;
			PositiveFloat angle_threshold = glm::radians(10.0f);
			std::set<BoundedRadians> snaps;
			StrictlyPositiveFloat strength = 1.0f;
			BoundedUnitInterval strength_offset = 0.2f;

			void set_uniformly_spaced_without_threshold(const size_t angles, float angle_offset = 0.0f)
			{
				snaps.clear();
				const float multiple = glm::two_pi<float>() / angles;
				for (size_t i = 0; i < angles; ++i)
					snaps.insert(i * multiple + angle_offset);
			}

			void set_uniformly_spaced(const size_t angles, float angle_offset = 0.0f)
			{
				snaps.clear();
				const float multiple = glm::two_pi<float>() / angles;
				for (size_t i = 0; i < angles; ++i)
					snaps.insert(i * multiple + angle_offset);
				angle_threshold = glm::pi<float>() / angles;
			}
		} angular_snapping;

		struct LinearSnapping
		{
			PositiveFloat speed_threshold = 0.1f;
			PositiveFloat position_threshold = 3.0f;
			float snap_width = 50.0f;
			float snap_offset = 0.0f;
			StrictlyPositiveFloat strength = 1.0f;
			BoundedUnitInterval strength_offset = 0.2f;
		};

		LinearSnapping linear_x_snapping, linear_y_snapping;

		float static_friction() const { return _static_friction; }
		float sqrt_static_friction() const { return _sqrt_static_friction; }
		void set_static_friction(float mu) { _static_friction.set(mu); _sqrt_static_friction.set(glm::sqrt(_static_friction)); }

		float kinetic_friction() const { return _kinetic_friction; }
		float sqrt_kinetic_friction() const { return _sqrt_kinetic_friction; }
		void set_kinetic_friction(float mu) { _kinetic_friction.set(mu); _sqrt_kinetic_friction.set(glm::sqrt(_kinetic_friction)); }

		float rolling_friction() const { return _rolling_friction; }
		float sqrt_rolling_friction() const { return _sqrt_rolling_friction; }
		void set_rolling_friction(float mu) { _rolling_friction.set(mu); _sqrt_rolling_friction.set(glm::sqrt(_rolling_friction)); }

		float restitution() const { return _restitution; }
		float sqrt_restitution() const { return _sqrt_restitution; }
		void set_restitution(float e) { _restitution.set(e); _sqrt_restitution.set(glm::sqrt(_restitution)); }

		float restitution_with(const Material& mat) const;
		float friction_with(const Material& mat, FrictionType friction_type) const;
	};

	typedef SmartHandle<Material> MaterialRef;
}
