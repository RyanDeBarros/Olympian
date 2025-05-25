#include "ConvexHull.h"

#include "core/types/Approximate.h"
#include "physics/collision/abstract/primitives/Common.h"

#include <algorithm>

namespace oly::acm2d
{
	ConvexHull ConvexHull::wrap(const math::Polygon2D& polygon)
	{
		if (polygon.size() <= 3)
			return { polygon };

		// Andrew's monotone chain

		std::vector<glm::vec2> sorted_polygon = polygon; // TODO version of wrap() that passes polygon by r-value so that it can be directly sorted here instead
		std::sort(sorted_polygon.begin(), sorted_polygon.end(), [](glm::vec2 lhs, glm::vec2 rhs) { return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y); });

		std::vector<glm::vec2> lower, upper;

		// lower hull
		for (glm::vec2 p : sorted_polygon)
		{
			while (lower.size() >= 2)
			{
				glm::vec2 a = lower[lower.size() - 2];
				glm::vec2 b = lower[lower.size() - 1];
				if (math::cross(b - a, p - a) <= 0.0f) // right turn
					lower.pop_back();
				else
					break;
			}
			lower.push_back(p);
		}

		// upper hull
		for (int i = (int)sorted_polygon.size() - 1; i >= 0; --i)
		{
			glm::vec2 p = sorted_polygon[i];
			while (upper.size() >= 2)
			{
				glm::vec2 a = upper[upper.size() - 2];
				glm::vec2 b = upper[upper.size() - 1];
				if (math::cross(b - a, p - a) <= 0.0f) // right turn
					upper.pop_back();
				else
					break;
			}
			upper.push_back(p);
		}

		// remove last points to avoid duplication
		lower.pop_back();
		upper.pop_back();

		lower.insert(lower.end(), upper.begin(), upper.end());

		return { lower };
	}

	std::pair<float, float> ConvexHull::projection_interval(const UnitVector2D& axis) const
	{
		return internal::polygon_projection_interval(points, axis);
	}

	UnitVector2D ConvexHull::edge_normal(size_t i) const
	{
		return internal::polygon_edge_normal(points, i);
	}

	glm::vec2 ConvexHull::deepest_point(const UnitVector2D& axis) const
	{
		return internal::polygon_deepest_point(points, axis);
	}
}
