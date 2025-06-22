#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Constants.h"
#include "core/base/Assert.h"
#include "physics/collision/Tolerance.h"

namespace oly::col2d::internal
{
	struct ProjectionCache
	{
		int index_min = 0;
		int index_max = 0;
		int index_deepest = 0;
	};

	template<typename Polygon>
	float polygon_projection_min(const Polygon& polygon, const UnitVector2D& axis, int& starting_index)
	{
		bool forward = true;
		int offset = 0;
		const size_t n = polygon.size();
		float min_proj = axis.dot(polygon[unsigned_mod(starting_index + offset++, n)]);
		while (offset < n)
		{
			float proj = axis.dot(polygon[unsigned_mod(starting_index + offset++, n)]);
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
		if (offset == n)
		{
			starting_index = unsigned_mod(starting_index - 1, n);
			return min_proj;
		}

		if (forward)
		{
			for (int i = offset; i < (int)n; ++i)
			{
				float proj = axis.dot(polygon[unsigned_mod(starting_index + i, n)]);
				if (proj < min_proj)
					min_proj = proj;
				else
				{
					starting_index = unsigned_mod(starting_index + i - 1, n);
					return min_proj;
				}
			}
			starting_index = unsigned_mod(starting_index - 1, n);
		}
		else
		{
			for (int i = (int)n - 1; i >= offset; --i)
			{
				float proj = axis.dot(polygon[unsigned_mod(starting_index + i, n)]);
				if (proj < min_proj)
					min_proj = proj;
				else
				{
					starting_index = unsigned_mod(starting_index + i + 1, n);
					return min_proj;
				}
			}
			starting_index = unsigned_mod(starting_index + offset, n);
		}
		return min_proj;
	}

	template<typename Polygon>
	float polygon_projection_min(const Polygon& polygon, const UnitVector2D& axis)
	{
		int starting_index = 0;
		return polygon_projection_min(polygon, axis, starting_index);
	}

	template<typename Polygon>
	float polygon_projection_min(const Polygon& polygon, const UnitVector2D& axis, ProjectionCache& cache)
	{
		return polygon_projection_min(polygon, axis, cache.index_min);
	}

	template<typename Polygon>
	float polygon_projection_max(const Polygon& polygon, const UnitVector2D& axis, int& starting_index)
	{
		bool forward = true;
		int offset = 0;
		const size_t n = polygon.size();
		float max_proj = axis.dot(polygon[unsigned_mod(starting_index + offset++, n)]);
		while (offset < n)
		{
			float proj = axis.dot(polygon[unsigned_mod(starting_index + offset++, n)]);
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
		if (offset == n)
		{
			starting_index = unsigned_mod(starting_index - 1, n);
			return max_proj;
		}

		if (forward)
		{
			for (int i = offset; i < (int)n; ++i)
			{
				float proj = axis.dot(polygon[unsigned_mod(starting_index + i, n)]);
				if (proj > max_proj)
					max_proj = proj;
				else
				{
					starting_index = unsigned_mod(starting_index + i - 1, n);
					return max_proj;
				}
			}
			starting_index = unsigned_mod(starting_index - 1, n);
		}
		else
		{
			for (int i = (int)n - 1; i >= offset; --i)
			{
				float proj = axis.dot(polygon[unsigned_mod(starting_index + i, n)]);
				if (proj > max_proj)
					max_proj = proj;
				else
				{
					starting_index = unsigned_mod(starting_index + i + 1, n);
					return max_proj;
				}
			}
			starting_index = unsigned_mod(starting_index + offset, n);
		}
		return max_proj;
	}

	template<typename Polygon>
	float polygon_projection_max(const Polygon& polygon, const UnitVector2D& axis)
	{
		int starting_index = 0;
		return polygon_projection_max(polygon, axis, starting_index);
	}

	template<typename Polygon>
	float polygon_projection_max(const Polygon& polygon, const UnitVector2D& axis, ProjectionCache& cache)
	{
		return polygon_projection_max(polygon, axis, cache.index_max);
	}

	template<typename Polygon>
	fpair polygon_projection_interval(const Polygon& polygon, const UnitVector2D& axis, int& starting_index_min, int& starting_index_max)
	{
		return { polygon_projection_min(polygon, axis, starting_index_min), polygon_projection_max(polygon, axis, starting_index_max) };
	}

	template<typename Polygon>
	fpair polygon_projection_interval(const Polygon& polygon, const UnitVector2D& axis)
	{
		int starting_index_min = 0, starting_index_max = 0;
		return polygon_projection_interval(polygon, axis, starting_index_min, starting_index_max);
	}

	template<typename Polygon>
	fpair polygon_projection_interval(const Polygon& polygon, const UnitVector2D& axis, ProjectionCache& cache)
	{
		return polygon_projection_interval(polygon, axis, cache.index_min, cache.index_max);
	}

	template<typename Polygon>
	UnitVector2D polygon_edge_normal(const Polygon& polygon, size_t i)
	{
		glm::vec2 edge = polygon[(i + 1) % polygon.size()] - polygon[i];
		return glm::vec2{ -edge.y, edge.x };
	}

	template<typename Polygon>
	glm::vec2 polygon_deepest_point(const Polygon& polygon, const UnitVector2D& axis, int& starting_index)
	{
		bool forward = true;
		int offset = 0;
		const size_t n = polygon.size();
		glm::vec2 deepest = polygon[unsigned_mod(starting_index + offset++, n)];
		float max_proj = axis.dot(deepest);
		int num_deepest_points = 1;
		
		while (offset < n)
		{
			glm::vec2 point = polygon[unsigned_mod(starting_index + offset++, n)];
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
		if (offset == n)
		{
			starting_index = unsigned_mod(starting_index - 1, n);
			return deepest /= (float)num_deepest_points;
		}

		if (forward)
		{
			int i = offset;
			while (i < (int)n)
			{
				glm::vec2 point = polygon[unsigned_mod(starting_index + i, n)];
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
				++i;
			}
			starting_index = unsigned_mod(starting_index + i - num_deepest_points, n);
		}
		else
		{
			int i = (int)n - 1;
			while (i >= offset)
			{
				glm::vec2 point = polygon[unsigned_mod(starting_index + i, n)];
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
				--i;
			}
			starting_index = unsigned_mod(starting_index + i + num_deepest_points, n);
		}
		return deepest /= (float)num_deepest_points;
	}

	template<typename Polygon>
	glm::vec2 polygon_deepest_point(const Polygon& polygon, const UnitVector2D& axis)
	{
		int starting_index = 0;
		return polygon_deepest_point(polygon, axis, starting_index);
	}

	template<typename Polygon>
	glm::vec2 polygon_deepest_point(const Polygon& polygon, const UnitVector2D& axis, ProjectionCache& cache)
	{
		return polygon_deepest_point(polygon, axis, cache.index_deepest);
	}
}
