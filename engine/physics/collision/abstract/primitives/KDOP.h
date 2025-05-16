#pragma once

#include "core/math/Geometry.h"
#include "physics/collision/abstract/primitives/ConvexHull.h"

#include <array>

namespace oly::acm2d
{
	template<size_t K_half>
	struct KDOP
	{
		static_assert(K_half >= 2, "kDOP must have degree at least 4.");

		std::array<glm::vec2, 2 * K_half> points;

		static std::array<glm::vec2, K_half> uniform_axes()
		{
			std::array<glm::vec2, K_half> axes;
			for (size_t i = 0; i < K_half; ++i)
				axes[i] = math::dir_vector(glm::two_pi<float>() * float(i) / float(K_half));
			return axes;
		}

		static KDOP<K_half> wrap(const math::Polygon2D& polygon)
		{
			std::array<glm::vec2, K_half> axes = uniform_axes();

			std::array<float, K_half> mins, maxs;
			mins.fill(FLT_MAX);
			maxs.fill(-FLT_MAX);

			for (glm::vec2 point : polygon.points)
			{
				for (size_t i = 0; i < K_half; ++i)
				{
					float v = glm::dot(point, axes[i]);
					mins[i] = std::min(mins[i], v);
					maxs[i] = std::max(maxs[i], v);
				}
			}

			math::Polygon2D cloud;
			cloud.points.reserve(2 * K_half);
			for (size_t i = 0; i < K_half; ++i)
			{
				cloud.points.push_back(axes[i] * mins[i]);
				cloud.points.push_back(axes[i] * maxs[i]);
			}

			ConvexHull hull = ConvexHull::wrap(cloud);
			KDOP<K_half> kdop;
			for (size_t i = 0; i < 2 * K_half; ++i)
				kdop.points[i] = hull.points[i];
			return kdop;
		}
	};

	template<size_t K_half, std::array<glm::vec2, K_half> Axes>
	struct CustomKDOP
	{
		static_assert(K_half >= 2, "kDOP must have degree at least 4.");

		std::array<glm::vec2, 2 * K_half> points;

		static KDOP<K_half> wrap(const math::Polygon2D& polygon)
		{
			std::array<float, K_half> mins, maxs;
			mins.fill(FLT_MAX);
			maxs.fill(-FLT_MAX);

			for (glm::vec2 point : polygon.points)
			{
				for (size_t i = 0; i < K_half; ++i)
				{
					float v = glm::dot(point, Axes[i]);
					mins[i] = std::min(mins[i], v);
					maxs[i] = std::max(maxs[i], v);
				}
			}

			math::Polygon2D cloud;
			cloud.points.reserve(2 * K_half);
			for (size_t i = 0; i < K_half; ++i)
			{
				cloud.points.push_back(Axes[i] * mins[i] / glm::dot(Axes[i], Axes[i]));
				cloud.points.push_back(Axes[i] * maxs[i] / glm::dot(Axes[i], Axes[i]));
			}

			ConvexHull hull = ConvexHull::wrap(cloud);
			KDOP<K_half> kdop;
			for (size_t i = 0; i < 2 * K_half; ++i)
				kdop.points[i] = hull.points[i];
			return kdop;
		}
	};
}
