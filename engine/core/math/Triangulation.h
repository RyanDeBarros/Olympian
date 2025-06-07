#pragma once

#include "core/math/Geometry.h"

namespace oly::math
{
	extern Triangulation triangulate(const Polygon2D& polygon, bool increasing = true, int starting_offset = 0, int ear_cycle = 0);
	extern glm::uint get_first_ear(const Polygon2D& polygon, int starting_offset = 0);
	extern std::vector<Triangulation> convex_decompose_triangulation(const Polygon2D& polygon);
	extern std::vector<Triangulation> convex_decompose_triangulation(const Polygon2D& polygon, const Triangulation& triangulation);
	extern std::vector<std::pair<Polygon2D, Triangulation>> convex_decompose_polygon(const Polygon2D& polygon);
	extern std::vector<std::pair<Polygon2D, Triangulation>> convex_decompose_polygon(const Polygon2D& polygon, const Triangulation& triangulation);
	extern std::vector<std::pair<Polygon2D, Triangulation>> decompose_polygon(const Polygon2D& polygon, const std::vector<Triangulation>& triangulations);
	extern std::vector<Polygon2D> convex_decompose_polygon_without_triangulation(const Polygon2D& polygon);
	extern std::vector<Polygon2D> convex_decompose_polygon_without_triangulation(const Polygon2D& polygon, const Triangulation& triangulation);
	extern std::vector<Polygon2D> decompose_polygon_without_triangulation(const Polygon2D& polygon, const std::vector<Triangulation>& triangulations);
}
