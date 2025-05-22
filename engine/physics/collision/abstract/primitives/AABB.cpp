#include "AABB.h"

namespace oly::acm2d
{
	AABB AABB::wrap(const math::Polygon2D& polygon)
	{
		AABB c{ .x1 = FLT_MAX, .x2 = -FLT_MAX, .y1 = FLT_MAX, .y2 = -FLT_MIN };
		for (glm::vec2 point : polygon.points)
		{
			c.x1 = std::min(c.x1, point.x);
			c.x2 = std::max(c.x2, point.x);
			c.y1 = std::min(c.y1, point.y);
			c.y2 = std::max(c.y2, point.y);
		}
		return c;
	}

	std::array<glm::vec2, 4> AABB::points() const
	{
		return {
			glm::vec2{ x1, y1 },
			glm::vec2{ x2, y1 },
			glm::vec2{ x2, y2 },
			glm::vec2{ x1, y2 }
		};
	}

	std::pair<float, float> AABB::projection_interval(glm::vec2 axis) const
	{
		std::pair<float, float> interval = { FLT_MAX, -FLT_MAX };
		for (glm::vec2 point : points())
		{
			float proj = glm::dot(point, axis);
			interval.first = std::min(interval.first, proj);
			interval.second = std::max(interval.second, proj);
		}
		return interval;
	}
}
