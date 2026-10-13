#include "ConvexHull.h"

#include "core/base/Approximate.h"

#include <algorithm>

namespace oly::col2d
{
    ConvexHull::ConvexHull(std::vector<glm::vec2> points)
        : points_(std::move(points))
    {
    }

	ConvexHull ConvexHull::wrap(const glm::vec2* polygon, size_t count)
	{
		math::Polygon2D poly(count);
		for (size_t i = 0; i < count; ++i)
			poly[i] = polygon[i];
		return wrap(poly);
	}

	ConvexHull ConvexHull::wrap(const math::Polygon2D& polygon)
	{
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

    const std::vector<glm::vec2>& ConvexHull::points() const
    {
        return points_;
    }

    std::vector<glm::vec2>& ConvexHull::set_points()
    {
        dirty_center_ = true;
        return points_;
    }
    
    size_t ConvexHull::size() const
    {
        return points_.size();
    }

	glm::vec2 ConvexHull::center() const
	{
		if (dirty_center_)
		{
			dirty_center_ = false;
			center_ = {};
			for (glm::vec2 p : points_)
				center_ += p;
			center_ /= (float)points_.size();
		}
		return center_;
	}

	fpair ConvexHull::projection_interval(const UnitVector2D& axis) const
	{
		return internal::polygon_projection_interval(points_, axis);
	}

	float ConvexHull::projection_min(const UnitVector2D& axis) const
	{
		return internal::polygon_projection_min(points_, axis, proj_cache_);
	}

	float ConvexHull::projection_max(const UnitVector2D& axis) const
	{
		return internal::polygon_projection_max(points_, axis, proj_cache_);
	}

	UnitVector2D ConvexHull::edge_normal(size_t i) const
	{
		return internal::polygon_edge_normal(points_, i);
	}

	ContactManifold ConvexHull::deepest_manifold(const UnitVector2D& axis) const
	{
		return internal::polygon_deepest_manifold(points_, axis, proj_cache_);
	}
}
