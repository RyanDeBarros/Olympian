#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Errors.h"
#include "core/math/Geometry.h"
#include "core/types/Approximate.h"
#include "physics/collision/abstract/primitives/ConvexHull.h"
#include "physics/collision/abstract/primitives/Common.h"

#include <array>
#include <string>
#include <vector>

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
			cloud.reserve(K);

			for (size_t i = 0; i < K_half; ++i)
			{
				cloud.push_back((glm::vec2)uniform_axis(i) * minima[i]);
				cloud.push_back((glm::vec2)uniform_axis(i) * maxima[i]);
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
			if (i == 0)
				return UnitVector2D::RIGHT;
			else if (i == 1)
				return UnitVector2D::UP;
			else
			{
				static constexpr size_t K_quadrant_1 = (K_half - 2) / 2;
				static constexpr size_t K_quadrant_4 = K_half - 2 - K_quadrant_1;
				i -= 2;
				if (i < K_quadrant_1)
					return UnitVector2D((i + 1) * glm::half_pi<float>() / K_quadrant_1);
				else
					return UnitVector2D(-int(i - K_quadrant_1 + 1) * glm::half_pi<float>() / K_quadrant_4);
			}
		}

		static KDOP<K_half> wrap(const math::Polygon2D& polygon)
		{
			std::array<float, K_half> minima;
			std::array<float, K_half> maxima;
			minima.fill(std::numeric_limits<float>::max());
			maxima.fill(std::numeric_limits<float>::lowest());

			for (glm::vec2 point : polygon)
			{
				for (size_t i = 0; i < K_half; ++i)
				{
					float v = uniform_axis(i).dot(point);
					minima[i] = std::min(minima[i], v);
					maxima[i] = std::max(maxima[i], v);
				}
			}

			return KDOP<K_half>(minima, maxima);
		}

		const std::array<glm::vec2, K>& points() const
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

	class CustomKDOP
	{
		std::vector<UnitVector2D> axes;
		std::vector<float> minima;
		std::vector<float> maxima;
		mutable std::vector<glm::vec2> _cache;
		mutable bool dirty = true;

	public:
		size_t get_k_half() const { return axes.size(); }
		const std::vector<UnitVector2D>& get_axes() const { return axes; }
		const std::vector<float>& get_minima() const { return minima; }
		const std::vector<float>& get_maxima() const { return maxima; }
		void set_k_half(size_t k_half)
		{
			if (k_half < 2)
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 4, but k_half provided is: " + std::to_string(k_half));
			axes.resize(k_half);
			minima.resize(k_half);
			maxima.resize(k_half);
			dirty = true;
		}
		float get_minimum(size_t i) const { return minima[i]; }
		float get_maximum(size_t i) const { return maxima[i]; }
		void set_minimum(size_t i, float minimum) { minima[i] = minimum; dirty = true; }
		void set_maximum(size_t i, float maximum) { maxima[i] = maximum; dirty = true; }
		void set_axes(const std::vector<UnitVector2D>& axes)
		{
			set_k_half(axes.size());
			for (size_t i = 0; i < get_k_half(); ++i)
				this->axes[i] = axes[i];
		}
		void set_extrema(const std::vector<std::pair<float, float>>& extrema)
		{
			set_k_half(extrema.size());
			for (size_t i = 0; i < get_k_half(); ++i)
			{
				minima[i] = extrema[i].first;
				maxima[i] = extrema[i].second;
			}
		}

		CustomKDOP() = default;
		CustomKDOP(const std::vector<UnitVector2D>& axes, const std::vector<std::pair<float, float>>& extrema)
		{
			if (axes.size() != extrema.size())
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have the same number of extrema as axes");

			set_k_half(axes.size());
			for (size_t i = 0; i < get_k_half(); ++i)
			{
				this->axes[i] = axes[i];
				minima[i] = extrema[i].first;
				maxima[i] = extrema[i].second;
			}
		}

	private:
		void recompute_cache() const
		{
			math::Polygon2D cloud;
			cloud.reserve(2 * get_k_half());

			for (size_t i = 0; i < get_k_half(); ++i)
			{
				cloud.push_back((glm::vec2)axes[i] * minima[i]);
				cloud.push_back((glm::vec2)axes[i] * maxima[i]);
			}

			ConvexHull hull = ConvexHull::wrap(cloud);
			for (size_t i = 0; i < 2 * get_k_half(); ++i)
				_cache[i] = hull.points[i];
		}

		const std::vector<glm::vec2>& cache() const
		{
			if (dirty)
			{
				dirty = false;
				recompute_cache();
			}
			return _cache;
		}

	public:
		static CustomKDOP wrap(const math::Polygon2D& polygon, const std::vector<UnitVector2D>& axes)
		{
			size_t k_half = axes.size();
			if (k_half < 2)
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 4, but k_half provided is: " + std::to_string(k_half));
			std::vector<std::pair<float, float>> extrema(k_half);
			std::fill_n(extrema.begin(), k_half, std::make_pair(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()));

			for (glm::vec2 point : polygon)
			{
				for (size_t i = 0; i < k_half; ++i)
				{
					float v = axes[i].dot(point);
					extrema[i].first = std::min(extrema[i].first, v);
					extrema[i].second = std::max(extrema[i].second, v);
				}
			}

			return CustomKDOP(axes, extrema);
		}

		const std::vector<glm::vec2>& points() const
		{
			return cache();
		}

		std::pair<float, float> projection_interval(const UnitVector2D& axis) const
		{
			for (size_t i = 0; i < get_k_half(); ++i)
			{
				if (approx(axis, axes[i]))
					return { minima[i], maxima[i] };
			}
			return internal::polygon_projection_interval(cache(), axis);
		}

		UnitVector2D edge_normal(size_t i) const
		{
			return axes[i];
		}

		glm::vec2 deepest_point(const UnitVector2D& axis) const
		{
			return internal::polygon_deepest_point(cache(), axis);
		}
	};
}
