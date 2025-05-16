#pragma once

#include "core/math/Geometry.h"

namespace oly::acm2d
{
	struct OBB
	{
		glm::vec2 center;
		float width, height;
		float rotation;

		float area() const { return width * height; }

		static OBB fast_wrap(const math::Polygon2D& polygon, unsigned int quadrant_steps = 5, unsigned int iterations = 2);
		// TODO static OBB slow_wrap(const math::Polygon2D& polygon); using rotating calipers method
	};
}
