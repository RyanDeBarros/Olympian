#include "Material.h"

namespace oly::physics
{
	static float factor_blend(FactorBlendOp op, float a, float b, float sqrt_a, float sqrt_b)
	{
		switch (op)
		{
		case FactorBlendOp::Minimum:
			return std::min(a, b);
		case FactorBlendOp::ArithmeticMean:
			return 0.5f * (a + b);
		case FactorBlendOp::GeometricMean:
			return sqrt_a * sqrt_b;
		case FactorBlendOp::Active:
			return a;
		}
		return 0.0f;
	}

	float Restitution::restitution_with(Restitution other) const
	{
		return factor_blend(mode, _value, other._value, _sqrt, other._sqrt);
	}

	float Friction::friction_with(Friction other, FrictionType type) const
	{
		switch (type)
		{
		case FrictionType::Static:
			return factor_blend(static_mode, _static_friction, other._static_friction, _sqrt_static_friction, other._sqrt_static_friction);
		case FrictionType::Kinetic:
			return factor_blend(kinetic_mode, _kinetic_friction, other._kinetic_friction, _sqrt_kinetic_friction, other._sqrt_kinetic_friction);
		case FrictionType::Rolling:
			return factor_blend(rolling_mode, _rolling_friction, other._rolling_friction, _sqrt_rolling_friction, other._sqrt_rolling_friction);
		}
		return 0.0f;
	}
}
