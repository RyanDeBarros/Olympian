#include "Geometry.h"

#include "core/math/Coordinates.h"
#include "core/base/SimpleMath.h"

namespace oly::math
{
	float cross(glm::vec2 u, glm::vec2 v)
	{
		return u.x * v.y - u.y * v.x;
	}

	float cross(glm::vec2 u, UnitVector2D v)
	{
		return u.x * v.y() - u.y * v.x();
	}

	float cross(UnitVector2D u, glm::vec2 v)
	{
		return u.x() * v.y - u.y() * v.x;
	}

	float cross(UnitVector2D u, UnitVector2D v)
	{
		return u.x() * v.y() - u.y() * v.x();
	}

	glm::vec2 triple_cross(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
	{
		float d = cross(p1, p2);
		return { -d * p3.y, d * p3.x };
	}

	float magnitude(glm::vec2 v)
	{
		return glm::sqrt(glm::dot(v, v));
	}

	float mag_sqrd(glm::vec2 v)
	{
		return glm::dot(v, v);
	}

	float inv_magnitude(glm::vec2 v)
	{
		return glm::inversesqrt(glm::dot(v, v));
	}

	glm::vec2 project(glm::vec2 point, glm::vec2 axis)
	{
		return axis * glm::dot(point, axis) / glm::dot(axis, axis);
	}

	float projection_distance(glm::vec2 point, glm::vec2 axis)
	{
		return glm::dot(point, axis) / magnitude(axis);
	}

	bool in_convex_sector(glm::vec2 u1, glm::vec2 u2, glm::vec2 v)
	{
		if (cross(u1, u2) >= 0.0f)
			return cross(u1, v) >= 0.0f && cross(v, u2) >= 0.0f;
		else
			return cross(u1, v) <= 0.0f && cross(v, u2) <= 0.0f;
	}

	float signed_area(const std::vector<glm::vec2>& points)
	{
		float signed_area = cross(points.back(), points[0]);
		for (size_t i = 1; i < points.size(); ++i)
			signed_area += cross(points[i - 1], points[i]);
		return 0.5f * signed_area;
	}

	bool point_in_triangle(glm::vec2 test, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
	{
		return math::Barycentric(math::Triangle2D{ p1, p2, p3 }, {}).inside();
	}

	math::Polygon2D clip_polygon(const math::Polygon2D& polygon, const UnitVector2D& axis, float maximum)
	{
		math::Polygon2D out;
		const int n = (int)polygon.size();
		for (int i = 0; i < n; ++i)
		{
			glm::vec2 curr = polygon[i];
			glm::vec2 prev = polygon[unsigned_mod(i - 1, n)];
			bool curr_in = below_zero(axis.dot(curr) - maximum);
			bool prev_in = below_zero(axis.dot(prev) - maximum);

			if (curr_in != prev_in)
				out.push_back(prev + (curr - prev) * (maximum - axis.dot(prev)) / axis.dot(curr - prev));
			if (curr_in)
				out.push_back(curr);
		}
		return out;
	}
}
