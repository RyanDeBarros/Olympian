#pragma once

#include "core/cmath/ColoredGeometry.h"

namespace oly::cmath
{
	struct Decompose
	{
		Polygon2DComposite operator()(const Polygon2D& polygon) const;
		Polygon2DComposite operator()(const Polygon2D& polygon, const math::Triangulation& triangulation) const;
		Polygon2DComposite operator()(const Polygon2D& polygon, const std::vector<math::Triangulation>& triangulations) const;
		Polygon2DComposite operator()(const math::Polygon2D& polygon) const;
	};
}
