#pragma once

#include "external/GLM.h"
#include "core/base/SimpleMath.h"

namespace oly
{
	template<typename genType = float>
	constexpr genType golden_ratio() { return glm::golden_ratio<genType>(); }

	template<typename genType = float>
	constexpr genType inv_golden_ratio() { return genType(1.0) / glm::golden_ratio<genType>(); }

	template<typename genType = float>
	constexpr size_t golden_iterations(genType error) { return roundi(glm::log(error) / glm::log(inv_golden_ratio())); }
}
