#pragma once

#include "core/cmath/ColoredGeometry.h"

namespace oly::cmath
{
	extern Polygon2DComposite convex_decompose_polygon(const Polygon2D& polygon);
	extern Polygon2DComposite convex_decompose_polygon(const Polygon2D& polygon, const math::Triangulation& triangulation);
	extern Polygon2DComposite decompose_polygon(const Polygon2D& polygon, const std::vector<math::Triangulation>& triangulations); // TODO use convex_decompose_polygon over composite_convex_decomposition in archetype generation / asset loading
	extern Polygon2DComposite composite_convex_decomposition(const std::vector<glm::vec2>& points);
}
