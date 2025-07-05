#pragma once

#include <limits>

#include "external/GLM.h"
#include "core/types/Meta.h"

namespace oly
{
	template<numeric T = float>
	constexpr T golden_ratio() { return glm::golden_ratio<T>(); }

	template<numeric T = float>
	constexpr T inv_golden_ratio() { return T(1.0) / glm::golden_ratio<T>(); }

	template<numeric T = float>
	constexpr size_t golden_iterations(T error) { return (size_t)roundf(glm::log(error) / glm::log(inv_golden_ratio())); }

	template<numeric T>
	constexpr T nmax() { return std::numeric_limits<T>::max(); }
}
