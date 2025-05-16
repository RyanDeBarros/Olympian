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
}
