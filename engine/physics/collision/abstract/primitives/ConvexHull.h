#pragma once

#include "core/math/Geometry.h"

namespace oly::acm2d
{
	struct ConvexHull
	{
		std::vector<glm::vec2> points;

		static ConvexHull wrap(const math::Polygon2D& polygon);

		std::pair<float, float> projection_interval(glm::vec2 axis) const;
	};
}
