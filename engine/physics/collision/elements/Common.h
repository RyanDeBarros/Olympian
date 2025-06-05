#pragma once

#include "core/base/UnitVector.h"

namespace oly::col2d::internal
{
	template<typename Polygon>
	std::pair<float, float> polygon_projection_interval(const Polygon& polygon, const UnitVector2D& axis)
	{
		std::pair<float, float> interval = { std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest() };
		for (glm::vec2 point : polygon)
		{
			float proj = axis.dot(point);
			interval.first = std::min(interval.first, proj);
			interval.second = std::max(interval.second, proj);
		}
		return interval;
	}

	template<typename Polygon>
	UnitVector2D polygon_edge_normal(const Polygon& polygon, size_t i)
	{
		glm::vec2 edge = polygon[(i + 1) % polygon.size()] - polygon[i];
		return glm::vec2{ -edge.y, edge.x };
	}

	template<typename Polygon>
	glm::vec2 polygon_deepest_point(const Polygon& polygon, const UnitVector2D& axis)
	{
		glm::vec2 deepest{};
		size_t num_deepest_points = 0;
		float max_depth = std::numeric_limits<float>::lowest();
		for (glm::vec2 point : polygon)
		{
			float proj = axis.dot(point);
			if (proj > max_depth)
			{
				max_depth = proj;
				deepest = point;
				num_deepest_points = 1;
			}
			else if (approx(proj, max_depth))
			{
				deepest += point;
				++num_deepest_points;
			}
			else
				break; // polygon is convex, so as soon as depth tapers off, it will never return to exceed max depth
		}
		return deepest /= (float)num_deepest_points;
	}
}
