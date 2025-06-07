#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Constants.h"
#include "core/base/Assert.h"

namespace oly::col2d::internal
{
	template<typename Polygon>
	float polygon_projection_min(const Polygon& polygon, const UnitVector2D& axis)
	{
		OLY_ASSERT(polygon.size() >= 3); // TODO add getter/setter points for ConvexHull that does this check so it only needs to be executed then
		float min_proj = axis.dot(polygon[0]);
		float proj = axis.dot(polygon[1]);
		if (proj < min_proj)
		{
			min_proj = proj;
			for (auto it = polygon.begin() + 2; it != polygon.end(); ++it)
			{
				float proj = axis.dot(*it);
				if (proj < min_proj)
					min_proj = proj;
				else
					return min_proj;
			}
		}
		else
		{
			auto rend = polygon.rend();
			--rend;
			--rend;
			for (auto it = polygon.rbegin(); it != rend; ++it)
			{
				float proj = axis.dot(*it);
				if (proj < min_proj)
					min_proj = proj;
				else
					return min_proj;
			}
		}
		return min_proj;
	}

	template<typename Polygon>
	float polygon_projection_max(const Polygon& polygon, const UnitVector2D& axis)
	{
		OLY_ASSERT(polygon.size() >= 3); // TODO add getter/setter points for ConvexHull that does this check so it only needs to be executed then
		float max_proj = axis.dot(polygon[0]);
		float proj = axis.dot(polygon[1]);
		if (proj > max_proj)
		{
			max_proj = proj;
			for (auto it = polygon.begin() + 2; it != polygon.end(); ++it)
			{
				float proj = axis.dot(*it);
				if (proj > max_proj)
					max_proj = proj;
				else
					return max_proj;
			}
		}
		else
		{
			auto rend = polygon.rend();
			--rend;
			--rend;
			for (auto it = polygon.rbegin(); it != rend; ++it)
			{
				float proj = axis.dot(*it);
				if (proj > max_proj)
					max_proj = proj;
				else
					return max_proj;
			}
		}
		return max_proj;
	}

	template<typename Polygon>
	std::pair<float, float> polygon_projection_interval(const Polygon& polygon, const UnitVector2D& axis)
	{
		return { polygon_projection_min(polygon, axis), polygon_projection_max(polygon, axis) };
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
		float max_depth = -nmax<float>();
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
