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
		BoundedUnitInterval _value;
		BoundedUnitInterval _sqrt;

	public:
		FactorBlendOp mode = FactorBlendOp::MINIMUM;

		Restitution(BoundedUnitInterval v = 0.3f) : _value(v), _sqrt(glm::sqrt(_value)) {}

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

	// TODO v6 density
	struct Material
	{
		Restitution restitution;
		Friction friction;
	};

	typedef SmartReference<Material> MaterialRef;
	// TODO v5 MaterialRegistry for Material assets. + SubMaterial assets. + every other ___Ref asset.
}
