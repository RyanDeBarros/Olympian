#pragma once

#include "core/math/Geometry.h"

namespace oly::acm2d
{
	struct Capsule
	{
		glm::vec2 center;
		float half_width, half_height, rotation;
	};
}
