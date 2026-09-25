#pragma once

#include "external/GLM.h"

#include <imp/traits.hpp>

#include <limits>

namespace oly
{
	template<imp::numeric T = float>
	constexpr T golden_ratio() { return glm::golden_ratio<T>(); }

	template<imp::numeric T = float>
	constexpr T inv_golden_ratio() { return T(1) / glm::golden_ratio<T>(); }

	template<imp::numeric T = float>
	constexpr T inv_log_inv_golden_ratio() { return T(1) / glm::log(inv_golden_ratio<T>()); }

	template<imp::numeric T>
	constexpr T nmax() { return std::numeric_limits<T>::max(); }
}
