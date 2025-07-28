#include "Material.h"

namespace oly::physics
{
	float Restitution::restitution_with(Restitution other) const
	{
		switch (mode)
		{
		case FactorBlendOp::MINIMUM:
			return std::min(_value, other._value);
		case FactorBlendOp::ARITHMETIC_MEAN:
			return 0.5f * (_value + other._value);
		case FactorBlendOp::GEOMETRIC_MEAN:
			return _sqrt * other._sqrt;
		case FactorBlendOp::ACTIVE:
			return _value;
		}
		return 0.0f;
	}

	float Friction::friction_with(Friction other, FrictionType type) const
	{
		switch (type)
		{
		case FrictionType::STATIC:
			switch (static_mode)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_static_friction, other._static_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_static_friction + other._static_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_static_friction * other._sqrt_static_friction;
			case FactorBlendOp::ACTIVE:
				return _static_friction;
			}
			break;
		case FrictionType::KINETIC:
			switch (kinetic_mode)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_kinetic_friction, other._kinetic_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_kinetic_friction + other._kinetic_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_kinetic_friction * other._sqrt_kinetic_friction;
			case FactorBlendOp::ACTIVE:
				return _kinetic_friction;
			}
			break;
		case FrictionType::ROLLING:
			switch (rolling_mode)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_rolling_friction, other._rolling_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_rolling_friction + other._rolling_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_rolling_friction * other._sqrt_rolling_friction;
			case FactorBlendOp::ACTIVE:
				return _rolling_friction;
			}
			break;
		}
		return 0.0f;
	}

	void AngularSnapping::set_uniformly_spaced_without_threshold(const size_t angles, float angle_offset)
	{
		snaps.clear();
		const float multiple = glm::two_pi<float>() / angles;
		for (size_t i = 0; i < angles; ++i)
			snaps.insert(i * multiple + angle_offset);
	}

	void AngularSnapping::set_uniformly_spaced(const size_t angles, float angle_offset)
	{
		snaps.clear();
		const float multiple = glm::two_pi<float>() / angles;
		for (size_t i = 0; i < angles; ++i)
			snaps.insert(i * multiple + angle_offset);
		angle_threshold = glm::pi<float>() / angles;
	}
}
