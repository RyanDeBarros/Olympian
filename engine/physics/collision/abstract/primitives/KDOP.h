#pragma once

#include "core/base/UnitVector.h"
#include "core/math/Geometry.h"
#include "physics/collision/abstract/primitives/ConvexHull.h"

#include <array>

namespace oly::acm2d
{
	template<size_t K_half>
	struct KDOP
	{
		static_assert(K_half >= 2, "kDOP must have degree at least 4.");

		std::array<UnitVector2D, 2 * K_half> points;

		static std::array<UnitVector2D, K_half> uniform_axes()
		{
			std::array<UnitVector2D, K_half> axes;
			for (size_t i = 0; i < K_half; ++i)
				axes[i] = UnitVector2D(glm::two_pi<float>() * float(i) / float(K_half));
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
			cloud.points.reserve(2 * K_half);
			for (size_t i = 0; i < K_half; ++i)
			{
				cloud.points.push_back((glm::vec2)axes[i] * mins[i]);
				cloud.points.push_back((glm::vec2)axes[i] * maxs[i]);
			}

			ConvexHull hull = ConvexHull::wrap(cloud);
			KDOP<K_half> kdop;
			for (size_t i = 0; i < 2 * K_half; ++i)
				kdop.points[i] = hull.points[i];
			return kdop;
		}
	};

	template<size_t K_half, std::array<UnitVector2D, K_half> Axes>
	struct CustomKDOP
	{
		static_assert(K_half >= 2, "Custom kDOP must have degree at least 4.");

		std::array<UnitVector2D, 2 * K_half> points;

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
			cloud.points.reserve(2 * K_half);
			for (size_t i = 0; i < K_half; ++i)
			{
				cloud.points.push_back((glm::vec2)Axes[i] * mins[i]);
				cloud.points.push_back((glm::vec2)Axes[i] * maxs[i]);
			}

			ConvexHull hull = ConvexHull::wrap(cloud);
			KDOP<K_half> kdop;
			for (size_t i = 0; i < 2 * K_half; ++i)
				kdop.points[i] = hull.points[i];
			return kdop;
		}
	};
}
