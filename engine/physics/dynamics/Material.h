#pragma once

#include "core/base/Parameters.h"
#include "core/types/Handle.h"

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
		BoundedFloat<0.0f, 1.0f> _restitution = 0.2f;
		BoundedFloat<0.0f, 1.0f> _sqrt_restitution = glm::sqrt(_restitution);

	public:
		PositiveFloat linear_drag = 0.0f;
		PositiveFloat angular_drag = 1.0f;

		struct
		{
			// linear penetration damping controls how much penetration motion into another collider is clamped.
			// At 0.0 - no clamping, which may cause clipping and significant friction due to high normal force.
			// At 1.0 - complete clamping, which may cause slight jitteriness or alternating colliding states.
			BoundedFloat<0.0f, 1.0f> linear_penetration = 0.5f;
			// angular teleportation damping controls how much corrective angular impulse is accumulated from collisions.
			// At 0.0 - no damping, which causes large angular bounces from collisions.
			// At 1.0 - full damping, which prevents any angular response to collisions.
			PowerInterval angular_teleportation = PowerInterval(0.85f);
			// angular teleportation values under jitter threshold are fully dampened.
			PositiveFloat angular_jitter_threshold = 0.001f;
		} collision_damping;

		struct
		{
			FactorBlendOp restitution = FactorBlendOp::MINIMUM;
			FactorBlendOp static_friction = FactorBlendOp::GEOMETRIC_MEAN;
			FactorBlendOp kinetic_friction = FactorBlendOp::GEOMETRIC_MEAN;
			FactorBlendOp rolling_friction = FactorBlendOp::GEOMETRIC_MEAN;
		} blending;

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

	typedef Handle<Material> MaterialRef;
}
