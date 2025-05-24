#pragma once

#include "core/base/UnitVector.h"

namespace oly::acm2d::internal
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
		float max_depth = std::numeric_limits<float>::lowest();
		for (glm::vec2 point : polygon)
		{
			float proj = axis.dot(point);
			if (proj > max_depth)
			{
				max_depth = proj;
				deepest = point;
			}
			else if (approx(proj, max_depth))
				deepest = 0.5f * (deepest + point);
		}
		return deepest;
	}
}
