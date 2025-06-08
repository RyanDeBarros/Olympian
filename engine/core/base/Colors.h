#pragma once

#include "external/GLM.h"

namespace oly::colors
{
	constexpr glm::vec4 BLACK		= { 0.0f, 0.0f, 0.0f, 1.0f };
	constexpr glm::vec4 WHITE		= { 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr glm::vec4 RED			= { 1.0f, 0.0f, 0.0f, 1.0f };
	constexpr glm::vec4 GREEN		= { 0.0f, 1.0f, 0.0f, 1.0f };
	constexpr glm::vec4 BLUE		= { 0.0f, 0.0f, 1.0f, 1.0f };
	constexpr glm::vec4 YELLOW		= { 1.0f, 1.0f, 0.0f, 1.0f };
	constexpr glm::vec4 MAGENTA		= { 1.0f, 0.0f, 1.0f, 1.0f };
	constexpr glm::vec4 CYAN		= { 0.0f, 1.0f, 1.0f, 1.0f };

	constexpr glm::vec4 alpha(float a) { return { 1.0f, 1.0f, 1.0f, a }; }
}
