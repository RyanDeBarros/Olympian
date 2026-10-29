#pragma once

#include "core/base/Approximate.h"

namespace oly::col2d
{
    constexpr double kLinearTolerance = 1e-4;

    template<typename T>
    inline bool approx(const T& a, const T& b)
    {
        return oly::approx(a, b, kLinearTolerance);
    }

    template<typename T>
    inline bool near_zero(const T& a)
    {
        return oly::near_zero(a, kLinearTolerance);
    }

    template<typename T>
    inline bool above_zero(const T& a)
    {
        return oly::above_zero(a, kLinearTolerance);
    }

    template<typename T>
    inline bool below_zero(const T& a)
    {
        return oly::below_zero(a, kLinearTolerance);
    }

    template<imp::numeric T>
    inline bool near_multiple(const T& a, const T& b)
    {
        return oly::near_multiple(a, b, kLinearTolerance);
    }
}
