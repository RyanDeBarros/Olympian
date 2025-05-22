#include "ConvexHull.h"

#include <algorithm>

namespace oly::acm2d
{
	ConvexHull ConvexHull::wrap(const math::Polygon2D& polygon)
	{
		if (polygon.points.size() <= 3)
			return { polygon.points };

		// Andrew's monotone chain

		std::vector<glm::vec2> sorted_polygon = polygon.points;
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

	std::pair<float, float> ConvexHull::projection_interval(glm::vec2 axis) const
	{
		std::pair<float, float> interval = { FLT_MAX, -FLT_MAX };
		for (glm::vec2 point : points)
		{
			float proj = glm::dot(point, axis);
			interval.first = std::min(interval.first, proj);
			interval.second = std::max(interval.second, proj);
		}
		return interval;
	}
}
