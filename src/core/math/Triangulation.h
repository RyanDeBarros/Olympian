#pragma once

#include "core/math/Shapes.h"
#include "core/math/ColoredGeometry.h"

namespace oly
{
	namespace cmath
	{
		extern Triangulation triangulate(const std::vector<glm::vec2>& polygon, bool increasing = true, int starting_offset = 0, int ear_cycle = 0);
		extern glm::uint get_first_ear(const std::vector<glm::vec2>& polygon, int starting_offset = 0);
		extern std::vector<Triangulation> convex_decompose_triangulation(const std::vector<glm::vec2>& polygon);
		extern std::vector<Triangulation> convex_decompose_triangulation(const std::vector<glm::vec2>& polygon, const Triangulation& triangulation);
		extern std::vector<std::pair<std::vector<glm::vec2>, Triangulation>> convex_decompose_polygon(const std::vector<glm::vec2>& polygon);
		extern std::vector<std::pair<std::vector<glm::vec2>, Triangulation>> convex_decompose_polygon(const std::vector<glm::vec2>& polygon, const Triangulation& triangulation);
		extern std::vector<std::pair<std::vector<glm::vec2>, Triangulation>> decompose_polygon(const std::vector<glm::vec2>& polygon, const std::vector<Triangulation>& triangulations);
		extern Polygon2DComposite composite_convex_decomposition(const std::vector<glm::vec2>& points);
	}

	// TODO pure math versions
}
