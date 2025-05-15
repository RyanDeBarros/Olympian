#pragma once

#include "core/math/Geometry.h"

namespace oly::acm2d
{
	struct AABB
	{
		float x1, x2, y1, y2;

		float area() const { return (x2 - x1) * (y2 - y1); }

		static AABB wrap(const math::Polygon2D& polygon);
	};
}
