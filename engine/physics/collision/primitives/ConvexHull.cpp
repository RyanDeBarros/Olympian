#include "ConvexHull.h"

#include "core/types/Approximate.h"
#include "physics/collision/primitives/Common.h"

#include <algorithm>

namespace oly::col2d
{
	ConvexHull ConvexHull::wrap(const math::Polygon2D& polygon)
	{
		if (polygon.size() <= 3)
			return { polygon };

		math::Polygon2D cpy = polygon;
		return wrap(cpy);
	}

	ConvexHull ConvexHull::wrap(math::Polygon2D& polygon)
	{
		if (polygon.size() <= 3)
			return { polygon };

		// Andrew's monotone chain

		std::sort(polygon.begin(), polygon.end(), [](glm::vec2 lhs, glm::vec2 rhs) { return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y); });

		std::vector<glm::vec2> lower, upper;

		// lower hull
		for (glm::vec2 p : polygon)
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
		for (int i = (int)polygon.size() - 1; i >= 0; --i)
		{
			glm::vec2 p = polygon[i];
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

		return { std::move(lower) };
	}

	glm::vec2 ConvexHull::center() const
	{
		if (dirty_center)
		{
			dirty_center = false;
			_center = {};
			for (glm::vec2 p : _points)
				_center += p;
			_center /= (float)_points.size();
		}
		return _center;
	}

	std::pair<float, float> ConvexHull::projection_interval(const UnitVector2D& axis) const
	{
		return internal::polygon_projection_interval(_points, axis);
	}

	UnitVector2D ConvexHull::edge_normal(size_t i) const
	{
		return internal::polygon_edge_normal(_points, i);
	}

	glm::vec2 ConvexHull::deepest_point(const UnitVector2D& axis) const
	{
		return internal::polygon_deepest_point(_points, axis);
	}
}
