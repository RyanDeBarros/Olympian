#pragma once

#include "core/base/Parameters.h"
#include "core/types/SmartReference.h"

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

	struct Restitution
	{
	private:
		BoundedUnitInterval _value = 0.2f;
		BoundedUnitInterval _sqrt = glm::sqrt(_value);

	public:
		FactorBlendOp mode = FactorBlendOp::MINIMUM;

		Restitution(BoundedUnitInterval v = 0.2f) : _value(v), _sqrt(glm::sqrt(_value)) {}

		float value() const { return _value; }
		float sqrt() const { return _sqrt; }
		void set_value(float e) { _value.set(e); _sqrt.set(glm::sqrt(_value)); }

		float restitution_with(Restitution other) const;

		static const Restitution FULL_BOUNCE;
		static const Restitution NO_BOUNCE;
	};

	inline const Restitution Restitution::FULL_BOUNCE = Restitution(1.0f);
	inline const Restitution Restitution::NO_BOUNCE = Restitution(0.0f);

	struct Friction
	{
	private:
		PositiveFloat _static_friction = 0.5f;
		PositiveFloat _sqrt_static_friction = glm::sqrt(_static_friction);
		PositiveFloat _kinetic_friction = 0.3f;
		PositiveFloat _sqrt_kinetic_friction = glm::sqrt(_kinetic_friction);
		PositiveFloat _rolling_friction = 0.4f;
		PositiveFloat _sqrt_rolling_friction = glm::sqrt(_rolling_friction);

	public:
		FactorBlendOp static_mode = FactorBlendOp::GEOMETRIC_MEAN;
		FactorBlendOp kinetic_mode = FactorBlendOp::GEOMETRIC_MEAN;
		FactorBlendOp rolling_mode = FactorBlendOp::GEOMETRIC_MEAN;

		float static_coeff() const { return _static_friction; }
		float static_sqrt() const { return _sqrt_static_friction; }
		void set_static_coeff(float mu) { _static_friction.set(mu); _sqrt_static_friction.set(glm::sqrt(_static_friction)); }

		float kinetic_coeff() const { return _kinetic_friction; }
		float kinetic_sqrt() const { return _sqrt_kinetic_friction; }
		void set_kinetic_coeff(float mu) { _kinetic_friction.set(mu); _sqrt_kinetic_friction.set(glm::sqrt(_kinetic_friction)); }

		float rolling_coeff() const { return _rolling_friction; }
		float rolling_sqrt() const { return _sqrt_rolling_friction; }
		void set_rolling_coeff(float mu) { _rolling_friction.set(mu); _sqrt_rolling_friction.set(glm::sqrt(_rolling_friction)); }

		float friction_with(Friction other, FrictionType type) const;
	};

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

	// LATER density
	struct Material
	{
		Restitution restitution;
		Friction friction;

		PositiveFloat linear_drag = 0.0f;
		PositiveFloat angular_drag = 1.0f;

		CollisionDamping collision_damping;
		AngularSnapping angular_snapping;
		LinearSnapping linear_x_snapping, linear_y_snapping;
	};

	typedef SmartReference<Material> MaterialRef;
	// TODO MaterialRegistry for Material assets
}
