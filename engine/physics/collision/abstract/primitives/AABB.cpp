#include "AABB.h"

#include "core/types/Approximate.h"
#include "physics/collision/abstract/primitives/Common.h"

namespace oly::acm2d
{
	AABB AABB::wrap(const math::Polygon2D& polygon)
	{
		AABB c{ .x1 = std::numeric_limits<float>::max(), .x2 = std::numeric_limits<float>::lowest(), .y1 = std::numeric_limits<float>::max(), .y2 = std::numeric_limits<float>::lowest() };
		for (glm::vec2 point : polygon)
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

	std::pair<float, float> AABB::projection_interval(const UnitVector2D& axis) const
	{
		auto pts = points();
		return internal::polygon_projection_interval(pts, axis);
	}

	glm::vec2 AABB::deepest_point(const UnitVector2D& axis) const
	{
		auto pts = points();
		return internal::polygon_deepest_point(pts, axis);
	}
}
