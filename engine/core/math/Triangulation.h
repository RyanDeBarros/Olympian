#pragma once

#include "core/math/Geometry.h"

namespace oly::math
{
	extern Triangulation triangulate(const std::vector<glm::vec2>& polygon, bool increasing = true, int starting_offset = 0, int ear_cycle = 0);
	extern glm::uint get_first_ear(const std::vector<glm::vec2>& polygon, int starting_offset = 0);
	extern std::vector<Triangulation> convex_decompose_triangulation(const std::vector<glm::vec2>& polygon);
	extern std::vector<Triangulation> convex_decompose_triangulation(const std::vector<glm::vec2>& polygon, const Triangulation& triangulation);
	extern std::vector<std::pair<std::vector<glm::vec2>, Triangulation>> convex_decompose_polygon(const std::vector<glm::vec2>& polygon);
	extern std::vector<std::pair<std::vector<glm::vec2>, Triangulation>> convex_decompose_polygon(const std::vector<glm::vec2>& polygon, const Triangulation& triangulation);
	extern std::vector<std::pair<std::vector<glm::vec2>, Triangulation>> decompose_polygon(const std::vector<glm::vec2>& polygon, const std::vector<Triangulation>& triangulations);
}
