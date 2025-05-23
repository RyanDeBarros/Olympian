#pragma once

#include "core/base/UnitVector.h"

namespace oly::acm2d
{
	struct HalfPlane
	{
		glm::vec2 origin;
		UnitVector2D direction;
	};
}
