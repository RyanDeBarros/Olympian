#pragma once

#include "core/types/Approximate.h"

namespace oly::col2d
{
	constexpr double LINEAR_TOLERANCE = 1e-4;

	template<typename T>
	inline bool approx(const T& a, const T& b)
	{
		return oly::approx(a, b, LINEAR_TOLERANCE);
	}

	template<typename T>
	inline bool near_zero(const T& a)
	{
		return oly::near_zero(a, LINEAR_TOLERANCE);
	}

	template<typename T>
	inline bool above_zero(const T& a)
	{
		return oly::above_zero(a, LINEAR_TOLERANCE);
	}

	template<typename T>
	inline bool below_zero(const T& a)
	{
		return oly::below_zero(a, LINEAR_TOLERANCE);
	}

	template<numeric T>
	inline bool near_multiple(const T& a, const T& b)
	{
		return oly::near_multiple(a, b, LINEAR_TOLERANCE);
	}
}
