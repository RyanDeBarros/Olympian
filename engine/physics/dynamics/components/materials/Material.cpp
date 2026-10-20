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

        default:
    		return 0.0f;
		}
	}

    Restitution::Restitution(BUInterval v)
        : value_(v)
        , sqrt_(glm::sqrt(value_))
    {
    }

    float Restitution::value() const
    {
        return value_;
    }

    float Restitution::sqrt() const
    {
        return sqrt_;
    }
    
    void Restitution::set_value(float e)
    {
        value_.set(e);
        sqrt_.set(glm::sqrt(value_));
    }

	float Restitution::restitution_with(Restitution other) const
	{
		return factor_blend(mode, value_, other.value_, sqrt_, other.sqrt_);
	}

    float Friction::static_coeff() const
    {
        return static_friction_;
    }

    float Friction::static_sqrt() const
    {
        return sqrt_static_friction_;
    }
    
    void Friction::set_static_coeff(float mu)
    {
        static_friction_.set(mu);
        sqrt_static_friction_.set(glm::sqrt(static_friction_));
    }

    float Friction::kinetic_coeff() const
    {
        return kinetic_friction_;
    }
    
    float Friction::kinetic_sqrt() const
    {
        return sqrt_kinetic_friction_;
    }
    
    void Friction::set_kinetic_coeff(float mu)
    {
        kinetic_friction_.set(mu);
        sqrt_kinetic_friction_.set(glm::sqrt(kinetic_friction_));
    }

    float Friction::rolling_coeff() const
    {
        return rolling_friction_;
    }
    
    float Friction::rolling_sqrt() const
    {
        return sqrt_rolling_friction_;
    }
    
    void Friction::set_rolling_coeff(float mu)
    {
        rolling_friction_.set(mu);
        sqrt_rolling_friction_.set(glm::sqrt(rolling_friction_));
    }

	float Friction::friction_with(Friction other, FrictionType type) const
	{
		switch (type)
		{
		case FrictionType::Static:
			return factor_blend(static_mode, static_friction_, other.static_friction_, sqrt_static_friction_, other.sqrt_static_friction_);

		case FrictionType::Kinetic:
			return factor_blend(kinetic_mode, kinetic_friction_, other.kinetic_friction_, sqrt_kinetic_friction_, other.sqrt_kinetic_friction_);

        case FrictionType::Rolling:
			return factor_blend(rolling_mode, rolling_friction_, other.rolling_friction_, sqrt_rolling_friction_, other.sqrt_rolling_friction_);

        default:
    		return 0.0f;
		}
	}
}
