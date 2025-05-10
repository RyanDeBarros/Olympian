#pragma once

#include "external/GLM.h"

namespace oly::math
{
	extern float cross(glm::vec2 u, glm::vec2 v);
	extern bool in_convex_sector(glm::vec2 u1, glm::vec2 u2, glm::vec2 test);
	extern float signed_area(const std::vector<glm::vec2>& points);
		
	template<typename T, enum glm::qualifier Q = glm::packed_highp>
	inline glm::vec<2, T, Q> reverse(glm::vec<2, T, Q> vec) { return { vec.y, vec.x }; }
	template<typename T, enum glm::qualifier Q = glm::packed_highp>
	inline glm::vec<3, T, Q> reverse(glm::vec<3, T, Q> vec) { return { vec.z, vec.y, vec.x }; }
	template<typename T, enum glm::qualifier Q = glm::packed_highp>
	inline glm::vec<4, T, Q> reverse(glm::vec<4, T, Q> vec) { return { vec.w, vec.z, vec.y, vec.x }; }
}
