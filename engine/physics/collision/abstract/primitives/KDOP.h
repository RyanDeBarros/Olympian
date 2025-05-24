#pragma once

#include "core/base/UnitVector.h"
#include "core/math/Geometry.h"
#include "physics/collision/abstract/primitives/ConvexHull.h"
#include "physics/collision/abstract/primitives/Common.h"

#include <array>

namespace oly::acm2d
{
	template<size_t K_half>
	struct KDOP
	{
		static_assert(K_half >= 2, "kDOP must have degree at least 4.");
		static const inline size_t K = 2 * K_half;

		std::array<UnitVector2D, K> points;

		static constexpr UnitVector2D uniform_axis(size_t i)
		{
			return UnitVector2D(glm::two_pi<float>() * float(i) / float(K_half));
		}

		static constexpr std::array<UnitVector2D, K_half> uniform_axes()
		{
			std::array<UnitVector2D, K_half> axes;
			for (size_t i = 0; i < K_half; ++i)
				axes[i] = uniform_axis(i);
			return axes;
		}

		static KDOP<K_half> wrap(const math::Polygon2D& polygon)
		{
			std::array<UnitVector2D, K_half> axes = uniform_axes();

			std::array<float, K_half> mins, maxs;
			mins.fill(std::numeric_limits<float>::max());
			maxs.fill(std::numeric_limits<float>::lowest());

			for (glm::vec2 point : polygon.points)
			{
				for (size_t i = 0; i < K_half; ++i)
				{
					float v = glm::dot(point, (glm::vec)axes[i]);
					mins[i] = std::min(mins[i], v);
					maxs[i] = std::max(maxs[i], v);
				}
			}

			math::Polygon2D cloud;
			cloud.points.reserve(K);
			for (size_t i = 0; i < K_half; ++i)
			{
				cloud.points.push_back((glm::vec2)axes[i] * mins[i]);
				cloud.points.push_back((glm::vec2)axes[i] * maxs[i]);
			}

			ConvexHull hull = ConvexHull::wrap(cloud);
			KDOP<K_half> kdop;
			for (size_t i = 0; i < K; ++i)
				kdop.points[i] = hull.points[i];
			return kdop;
		}

		std::pair<float, float> projection_interval(const UnitVector2D& axis) const
		{
			return internal::polygon_projection_interval(points, axis);
		}

		UnitVector2D edge_normal(size_t i) const
		{
			return internal::polygon_edge_normal(points, i);
		}
		
		glm::vec2 deepest_point(const UnitVector2D& axis) const
		{
			return internal::polygon_deepest_point(points, axis);
		}
	};

	template<size_t K_half, std::array<UnitVector2D, K_half> Axes>
	struct CustomKDOP
	{
		static_assert(K_half >= 2, "Custom kDOP must have degree at least 4.");
		static const inline size_t K = 2 * K_half;

		std::array<UnitVector2D, K> points;

		static KDOP<K_half> wrap(const math::Polygon2D& polygon)
		{
			std::array<float, K_half> mins, maxs;
			mins.fill(std::numeric_limits<float>::max());
			maxs.fill(std::numeric_limits<float>::lowest());

			for (glm::vec2 point : polygon.points)
			{
				for (size_t i = 0; i < K_half; ++i)
				{
					float v = glm::dot(point, (glm::vec2)Axes[i]);
					mins[i] = std::min(mins[i], v);
					maxs[i] = std::max(maxs[i], v);
				}
			}

			math::Polygon2D cloud;
			cloud.points.reserve(K);
			for (size_t i = 0; i < K_half; ++i)
			{
				cloud.points.push_back((glm::vec2)Axes[i] * mins[i]);
				cloud.points.push_back((glm::vec2)Axes[i] * maxs[i]);
			}

			ConvexHull hull = ConvexHull::wrap(cloud);
			KDOP<K_half> kdop;
			for (size_t i = 0; i < K; ++i)
				kdop.points[i] = hull.points[i];
			return kdop;
		}

		std::pair<float, float> projection_interval(const UnitVector2D& axis) const
		{
			return internal::polygon_projection_interval(points, axis);
		}

		UnitVector2D edge_normal(size_t i) const
		{
			return internal::polygon_edge_normal(points, i);
		}

		glm::vec2 deepest_point(const UnitVector2D& axis) const
		{
			return internal::polygon_deepest_point(points, axis);
		}
	};
}
