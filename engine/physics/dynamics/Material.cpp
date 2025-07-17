#include "Material.h"

namespace oly::physics
{
	float Material::restitution_with(const Material& mat) const
	{
		switch (blending.restitution)
		{
		case FactorBlendOp::MINIMUM:
			return std::min(_restitution, mat._restitution);
		case FactorBlendOp::ARITHMETIC_MEAN:
			return 0.5f * (_restitution + mat._restitution);
		case FactorBlendOp::GEOMETRIC_MEAN:
			return _sqrt_restitution * mat._sqrt_restitution;
		case FactorBlendOp::ACTIVE:
			return _restitution;
		}
		return 0.0f;
	}

	float Material::friction_with(const Material& mat, FrictionType friction_type) const
	{
		if (friction_type == FrictionType::STATIC)
		{
			switch (blending.static_friction)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_static_friction, mat._static_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_static_friction + mat._static_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_static_friction * mat._sqrt_static_friction;
			case FactorBlendOp::ACTIVE:
				return _static_friction;
			}
		}
		else if (friction_type == FrictionType::KINETIC)
		{
			switch (blending.kinetic_friction)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_kinetic_friction, mat._kinetic_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_kinetic_friction + mat._kinetic_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_kinetic_friction * mat._sqrt_kinetic_friction;
			case FactorBlendOp::ACTIVE:
				return _kinetic_friction;
			}
		}
		else if (friction_type == FrictionType::ROLLING)
		{
			switch (blending.rolling_friction)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_rolling_friction, mat._rolling_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_rolling_friction + mat._rolling_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_rolling_friction * mat._sqrt_rolling_friction;
			case FactorBlendOp::ACTIVE:
				return _rolling_friction;
			}
		}
		return 0.0f;
	}
}
