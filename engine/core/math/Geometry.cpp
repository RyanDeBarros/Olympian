#include "Geometry.h"

namespace oly::math
{
	float cross(glm::vec2 u, glm::vec2 v)
	{
		return u.x * v.y - u.y * v.x;
	}

	bool in_convex_sector(glm::vec2 u1, glm::vec2 u2, glm::vec2 test)
	{
		if (cross(u1, u2) >= 0.0f)
			return cross(u1, test) >= 0.0f && cross(test, u2) >= 0.0f;
		else
			return cross(u1, test) <= 0.0f && cross(test, u2) <= 0.0f;
	}

	float signed_area(const std::vector<glm::vec2>& points)
	{
		float signed_area = cross(points.back(), points[0]);
		for (size_t i = 1; i < points.size(); ++i)
			signed_area += cross(points[i - 1], points[i]);
		return 0.5f * signed_area;
	}
}
