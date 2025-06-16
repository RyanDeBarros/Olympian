#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Constants.h"
#include "core/base/Assert.h"
#include "physics/collision/Tolerance.h"

namespace oly::col2d::internal
{
	template<typename Polygon>
	float polygon_projection_min(const Polygon& polygon, const UnitVector2D& axis)
	{
		bool forward = true;
		int offset = 0;
		float min_proj = axis.dot(polygon[offset++]);
		while (offset < polygon.size())
		{
			float proj = axis.dot(polygon[offset++]);
			if (proj < min_proj)
			{
				min_proj = proj;
				forward = true;
				break;
			}
			else if (proj > min_proj)
			{
				forward = false;
				break;
			}
		}
		if (offset == polygon.size())
			return min_proj;

		if (forward)
		{
			for (int i = offset; i < (int)polygon.size(); ++i)
			{
				float proj = axis.dot(polygon[i]);
				if (proj < min_proj)
					min_proj = proj;
				else
					return min_proj;
			}
		}
		else
		{
			for (int i = (int)polygon.size() - 1; i >= offset; --i)
			{
				float proj = axis.dot(polygon[i]);
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
		bool forward = true;
		int offset = 0;
		float max_proj = axis.dot(polygon[offset++]);
		while (offset < polygon.size())
		{
			float proj = axis.dot(polygon[offset++]);
			if (proj > max_proj)
			{
				max_proj = proj;
				forward = true;
				break;
			}
			else if (proj < max_proj)
			{
				forward = false;
				break;
			}
		}
		if (offset == polygon.size())
			return max_proj;

		if (forward)
		{
			for (int i = offset; i < (int)polygon.size(); ++i)
			{
				float proj = axis.dot(polygon[i]);
				if (proj > max_proj)
					max_proj = proj;
				else
					return max_proj;
			}
		}
		else
		{
			for (int i = (int)polygon.size() - 1; i >= offset; --i)
			{
				float proj = axis.dot(polygon[i]);
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
		bool forward = true;
		int offset = 0;
		glm::vec2 deepest = polygon[offset++];
		float max_proj = axis.dot(deepest);
		int num_deepest_points = 1;
		
		while (offset < polygon.size())
		{
			glm::vec2 point = polygon[offset++];
			float proj = axis.dot(point);
			if (approx(proj, max_proj))
			{
				deepest += point;
				++num_deepest_points;
			}
			else if (proj > max_proj)
			{
				max_proj = proj;
				deepest = point;
				num_deepest_points = 1;
				forward = true;
				break;
			}
			else
			{
				forward = false;
				break;
			}
		}
		if (offset == polygon.size())
			return deepest /= (float)num_deepest_points;

		if (forward)
		{
			for (int i = offset; i < (int)polygon.size(); ++i)
			{
				glm::vec2 point = polygon[i];
				float proj = axis.dot(point);
				if (approx(proj, max_proj))
				{
					deepest += point;
					++num_deepest_points;
				}
				else if (proj > max_proj)
				{
					max_proj = proj;
					deepest = point;
					num_deepest_points = 1;
				}
				else
					break;
			}
		}
		else
		{
			for (int i = (int)polygon.size() - 1; i >= offset; --i)
			{
				glm::vec2 point = polygon[i];
				float proj = axis.dot(point);
				if (approx(proj, max_proj))
				{
					deepest += point;
					++num_deepest_points;
				}
				else if (proj > max_proj)
				{
					max_proj = proj;
					deepest = point;
					num_deepest_points = 1;
				}
				else
					break;
			}
		}
		return deepest /= (float)num_deepest_points;
	}
}
