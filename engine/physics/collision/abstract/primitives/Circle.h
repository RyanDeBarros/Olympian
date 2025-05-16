#pragma once

#include "core/math/Geometry.h"

namespace oly::acm2d
{
	struct Circle
	{
		glm::vec2 center;
		float radius;

		float area() const { return glm::pi<float>() * radius * radius; }

		static Circle fast_wrap(const math::Polygon2D& polygon);
	};
}
