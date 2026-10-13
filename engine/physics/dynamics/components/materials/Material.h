#pragma once

#include "core/base/Parameters.h"
#include "core/base/SmartReference.h"

namespace oly::physics
{
	enum class FrictionType
	{
		Static,
		Kinetic,
		Rolling
	};

	enum class FactorBlendOp
	{
		Minimum,
		GeometricMean,
		ArithmeticMean,
		Active
	};

	struct Restitution
	{
	private:
		BoundedUnitInterval value_;
		BoundedUnitInterval sqrt_;

	public:
		FactorBlendOp mode = FactorBlendOp::Minimum;

        Restitution(BoundedUnitInterval v = 0.3f);

        float value() const;
		float sqrt() const;
        void set_value(float e);

		float restitution_with(Restitution other) const;

		static const Restitution kFullBounce;
		static const Restitution kNoBounce;
	};

	inline const Restitution Restitution::kFullBounce = Restitution(1.0f);
	inline const Restitution Restitution::kNoBounce = Restitution(0.0f);

	struct Friction
	{
	private:
		PositiveFloat static_friction_ = 0.5f;
		PositiveFloat sqrt_static_friction_ = glm::sqrt(static_friction_);
		PositiveFloat kinetic_friction_ = 0.3f;
		PositiveFloat sqrt_kinetic_friction_ = glm::sqrt(kinetic_friction_);
		PositiveFloat rolling_friction_ = 0.4f;
		PositiveFloat sqrt_rolling_friction_ = glm::sqrt(rolling_friction_);

	public:
		FactorBlendOp static_mode = FactorBlendOp::GeometricMean;
		FactorBlendOp kinetic_mode = FactorBlendOp::GeometricMean;
		FactorBlendOp rolling_mode = FactorBlendOp::GeometricMean;

        float static_coeff() const;
		float static_sqrt() const;
        void set_static_coeff(float mu);

		float kinetic_coeff() const;
		float kinetic_sqrt() const;
		void set_kinetic_coeff(float mu);

		float rolling_coeff() const;
		float rolling_sqrt() const;
		void set_rolling_coeff(float mu);

		float friction_with(Friction other, FrictionType type) const;
	};

	struct Material
	{
		Restitution restitution;
		Friction friction;
	};

	using MaterialRef = SmartReference<Material>;
}
