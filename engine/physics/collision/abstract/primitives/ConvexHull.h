#pragma once

#include "core/base/UnitVector.h"
#include "core/math/Geometry.h"

namespace oly::acm2d
{
	struct ConvexHull
	{
		std::vector<glm::vec2> points;

		static ConvexHull wrap(const math::Polygon2D& polygon);
		static ConvexHull wrap(math::Polygon2D& polygon);

		std::pair<float, float> projection_interval(const UnitVector2D& axis) const;
		UnitVector2D edge_normal(size_t i) const;
		glm::vec2 deepest_point(const UnitVector2D& axis) const;
	};
}
