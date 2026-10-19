#pragma once

#include "core/base/Approximate.h"
#include "core/base/SimpleMath.h"

namespace oly
{
    template<typename T, T Min, T Max>
    struct BoundedValue
    {
        static_assert(Min < Max, "BoundedValue minimum must be less than maximum.");

    private:
        T val_;

    public:
        BoundedValue(T val = Min)
        {
            set(val);
        }

        BoundedValue<T, Min, Max>& operator=(T v)
        {
            set(v);
            return *this;
        }

        operator T() const
        {
            return get();
        }

        T get() const
        {
            return val_;
        }

        void set(T v)
        {
            val_ = glm::clamp(v, Min, Max);
        }
    };

    template<typename T, T Min, T Max>
    struct Distance<BoundedValue<T, Min, Max>>
    {
        double operator()(const BoundedValue<T, Min, Max>& a, const BoundedValue<T, Min, Max>& b) const
        {
            return static_cast<double>(glm::abs(a - b));
        }
    };

    template<typename T, T Min, T Max>
    struct ToleranceImpl<BoundedValue<T, Min, Max>>
    {
        static constexpr double kTol = 1e-7;
    };

    template<float Min, float Max>
    using BoundedFloat = BoundedValue<float, Min, Max>;
    template<int Min, int Max>
    using BoundedInt = BoundedValue<int, Min, Max>;
    template<unsigned int Min, unsigned int Max>
    using BoundedUInt = BoundedValue<unsigned int, Min, Max>;

    // TODO rename to pfloat and move to imp
    using PositiveFloat = BoundedFloat<0.0f, nmax<float>()>;
    // TODO rename to spfloat and move to imp
    using StrictlyPositiveFloat = BoundedFloat<0.000001f, nmax<float>()>;
    using BoundedUnitInterval = BoundedFloat<0.0f, 1.0f>;
    using BoundedRadians = BoundedFloat<0.0f, glm::two_pi<float>()>;

    struct PowerInterval
    {
    private:
        BoundedUnitInterval inner_ = 0.5f;
        BoundedUnitInterval outer_ = 0.5f;
        BoundedUnitInterval inner_midpoint_ = 0.5f;
        PositiveFloat power_ = 1.0f;

    public:
        PowerInterval(float inner_midpoint = 0.5f, float outer = 0.5f);

        PowerInterval& operator=(float outer);

        float inner() const;
        float outer() const;
        void set_outer(float outer);

        float inner_midpoint() const;
        float power() const;
        void set_inner_midpoint(float midpoint);
        void set_power(float power);

    private:
        void sync_inner();
    };

    template<>
    struct Distance<PowerInterval>
    {
        double operator()(const PowerInterval& a, const PowerInterval& b) const;
    };

    template<>
    struct ToleranceImpl<PowerInterval>
    {
        static constexpr double kTol = 1e-7;
    };
}
