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

	private:
		std::array<float, K_half> minima;
		std::array<float, K_half> maxima;
		mutable std::array<glm::vec2, K> _cache;
		mutable bool dirty = true;

	public:
		const std::array<float, K_half>& get_minima() const { return minima; }
		const std::array<float, K_half>& get_maxima() const { return maxima; }
		void set_minima(const std::array<float, K_half>& min) { minima = min; dirty = true; }
		void set_maxima(const std::array<float, K_half>& max) { maxima = max; dirty = true; }
		float get_minimum(size_t i) const { return minima[i]; }
		float get_maximum(size_t i) const { return maxima[i]; }
		void set_minimum(size_t i, float minimum) { minima[i] = minimum; dirty = true; }
		void set_maximum(size_t i, float maximum) { maxima[i] = maximum; dirty = true; }
		
		KDOP() = default;
		KDOP(const std::array<float, K_half>& minima, const std::array<float, K_half>& maxima) : minima(minima), maxima(maxima) { dirty = true; }

	private:
		void recompute_cache() const
		{
			math::Polygon2D cloud;
			cloud.points.reserve(K);

			for (size_t i = 0; i < K_half; ++i)
			{
				cloud.points.push_back((glm::vec2)uniform_axis(i) * minima[i]);
				cloud.points.push_back((glm::vec2)uniform_axis(i) * maxima[i]);
			}

			ConvexHull hull = ConvexHull::wrap(cloud);
			for (size_t i = 0; i < K; ++i)
				_cache[i] = hull.points[i];
		}

		const std::array<glm::vec2, K>& cache() const
		{
			if (dirty)
			{
				dirty = false;
				recompute_cache();
			}
			return _cache;
		}

	public:
		static constexpr UnitVector2D uniform_axis(size_t i)
		{
			return UnitVector2D(glm::two_pi<float>() * float(i) / float(K_half));
		}

		static KDOP<K_half> wrap(const math::Polygon2D& polygon)
		{
			std::array<float, K_half> minima;
			std::array<float, K_half> maxima;
			minima.fill(std::numeric_limits<float>::max());
			maxima.fill(std::numeric_limits<float>::lowest());

			for (glm::vec2 point : polygon.points)
			{
				for (size_t i = 0; i < K_half; ++i)
				{
					float v = glm::dot(point, (glm::vec)uniform_axis(i));
					minima[i] = std::min(minima[i], v);
					maxima[i] = std::max(maxima[i], v);
				}
			}

			return KDOP<K_half>(minima, maxima);
		}

		const std::array<glm::vec2, K>& get_polygon() const
		{
			return cache();
		}

		std::pair<float, float> projection_interval(const UnitVector2D& axis) const
		{
			for (size_t i = 0; i < K_half; ++i)
			{
				if (approx(axis, uniform_axis(i)))
					return { minima[i], maxima[i] };
			}
			return internal::polygon_projection_interval(cache(), axis);
		}

		UnitVector2D edge_normal(size_t i) const
		{
			return uniform_axis(i);
		}
		
		glm::vec2 deepest_point(const UnitVector2D& axis) const
		{
			return internal::polygon_deepest_point(cache(), axis);
		}
	};

	template<size_t K_half, std::array<UnitVector2D, K_half> Axes>
	struct CustomKDOP
	{
		static_assert(K_half >= 2, "kDOP must have degree at least 4.");
		static const inline size_t K = 2 * K_half;

	private:
		std::array<float, K_half> minima;
		std::array<float, K_half> maxima;
		mutable std::array<glm::vec2, K> _cache;
		mutable bool dirty = true;

	public:
		const std::array<float, K_half>& get_minima() const { return minima; }
		const std::array<float, K_half>& get_maxima() const { return maxima; }
		void set_minima(const std::array<float, K_half>& min) { minima = min; dirty = true; }
		void set_maxima(const std::array<float, K_half>& max) { maxima = max; dirty = true; }
		float get_minimum(size_t i) const { return minima[i]; }
		float get_maximum(size_t i) const { return maxima[i]; }
		void set_minimum(size_t i, float minimum) { minima[i] = minimum; dirty = true; }
		void set_maximum(size_t i, float maximum) { maxima[i] = maximum; dirty = true; }

		CustomKDOP() = default;
		CustomKDOP(const std::array<float, K_half>& minima, const std::array<float, K_half>& maxima) : minima(minima), maxima(maxima) { dirty = true; }

	private:
		void recompute_cache() const
		{
			math::Polygon2D cloud;
			cloud.points.reserve(K);

			for (size_t i = 0; i < K_half; ++i)
			{
				cloud.points.push_back((glm::vec2)Axes[i] * minima[i]);
				cloud.points.push_back((glm::vec2)Axes[i] * maxima[i]);
			}

			ConvexHull hull = ConvexHull::wrap(cloud);
			for (size_t i = 0; i < K; ++i)
				_cache[i] = hull.points[i];
		}

		const std::array<glm::vec2, K>& cache() const
		{
			if (dirty)
			{
				dirty = false;
				recompute_cache();
			}
			return _cache;
		}

	public:
		static KDOP<K_half> wrap(const math::Polygon2D& polygon)
		{
			std::array<float, K_half> minima;
			std::array<float, K_half> maxima;
			minima.fill(std::numeric_limits<float>::max());
			maxima.fill(std::numeric_limits<float>::lowest());

			for (glm::vec2 point : polygon.points)
			{
				for (size_t i = 0; i < K_half; ++i)
				{
					float v = glm::dot(point, (glm::vec)Axes[i]);
					minima[i] = std::min(minima[i], v);
					maxima[i] = std::max(maxima[i], v);
				}
			}

			return KDOP<K_half>(minima, maxima);
		}

		const std::array<glm::vec2, K>& get_polygon() const
		{
			return cache();
		}

		std::pair<float, float> projection_interval(const UnitVector2D& axis) const
		{
			for (size_t i = 0; i < K_half; ++i)
			{
				if (approx(axis, Axes[i]))
					return { minima[i], maxima[i] };
			}
			return internal::polygon_projection_interval(cache(), axis);
		}

		UnitVector2D edge_normal(size_t i) const
		{
			return Axes[i];
		}

		glm::vec2 deepest_point(const UnitVector2D& axis) const
		{
			return internal::polygon_deepest_point(cache(), axis);
		}
	};
}
