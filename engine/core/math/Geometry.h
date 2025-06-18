#pragma once

#include "external/GLM.h"
#include "core/base/UnitVector.h"

#include <vector>

namespace oly::math
{
	extern float cross(glm::vec2 u, glm::vec2 v);
	extern float cross(glm::vec2 u, UnitVector2D v);
	extern float cross(UnitVector2D u, glm::vec2 v);
	extern float cross(UnitVector2D u, UnitVector2D v);
	extern glm::vec2 triple_cross(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	extern float magnitude(glm::vec2 v);
	extern float mag_sqrd(glm::vec2 v);
	extern float inv_magnitude(glm::vec2 v);
	extern glm::vec2 project(glm::vec2 point, glm::vec2 axis);
	extern float projection_distance(glm::vec2 point, glm::vec2 axis);
	extern bool in_convex_sector(glm::vec2 u1, glm::vec2 u2, glm::vec2 v);
	extern float signed_area(const std::vector<glm::vec2>& points);
	inline bool point_in_triangle(glm::vec2 test, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	extern glm::vec2 closest_point_on_line_segment(glm::vec2 ref, glm::vec2 a, glm::vec2 b);
		
	template<typename T>
	inline T lerp(const T& t1, const T& t2, float alpha)
	{
		return t1 + alpha * (t2 - t1);
	}

	template<typename T, enum glm::qualifier Q = glm::packed_highp>
	inline glm::vec<2, T, Q> reverse(glm::vec<2, T, Q> vec) { return { vec.y, vec.x }; }
	template<typename T, enum glm::qualifier Q = glm::packed_highp>
	inline glm::vec<3, T, Q> reverse(glm::vec<3, T, Q> vec) { return { vec.z, vec.y, vec.x }; }
	template<typename T, enum glm::qualifier Q = glm::packed_highp>
	inline glm::vec<4, T, Q> reverse(glm::vec<4, T, Q> vec) { return { vec.w, vec.z, vec.y, vec.x }; }

	typedef std::vector<glm::vec2> Polygon2D;

	extern math::Polygon2D clip_polygon(const math::Polygon2D& polygon, const UnitVector2D& axis, float maximum);
}
