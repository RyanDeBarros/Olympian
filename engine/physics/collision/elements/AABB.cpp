#include "AABB.h"

#include "core/types/Approximate.h"
#include "physics/collision/elements/Common.h"

namespace oly::col2d
{
	AABB AABB::wrap(const glm::vec2* polygon, size_t count)
	{
		AABB c = DEFAULT;
		for (size_t i = 0; i < count; ++i)
		{
			glm::vec2 point = polygon[i];
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
