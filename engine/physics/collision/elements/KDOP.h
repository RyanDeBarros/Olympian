#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Errors.h"
#include "core/math/Geometry.h"
#include "core/types/Approximate.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/Common.h"

#include <array>
#include <string>
#include <vector>

namespace oly::col2d
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
		mutable glm::vec2 _center{};
		mutable bool dirty_cache = true;
		mutable bool dirty_center = true;

	public:
		const std::array<float, K_half>& get_minima() const { return minima; }
		const std::array<float, K_half>& get_maxima() const { return maxima; }
		void set_minima(const std::array<float, K_half>& min) { minima = min; dirty_cache = true; dirty_center = true; }
		void set_maxima(const std::array<float, K_half>& max) { maxima = max; dirty_cache = true; dirty_center = true; }
		float get_minimum(size_t i) const { return minima[i]; }
		float get_maximum(size_t i) const { return maxima[i]; }
		void set_minimum(size_t i, float minimum) { minima[i] = minimum; dirty_cache = true; dirty_center = true; }
		void set_maximum(size_t i, float maximum) { maxima[i] = maximum; dirty_cache = true; dirty_center = true; }
		
		KDOP() = default;
		KDOP(const std::array<float, K_half>& minima, const std::array<float, K_half>& maxima) : minima(minima), maxima(maxima) { dirty_cache = true; dirty_center = true; }
		
		void fill_invalid()
		{
			for (size_t i = 0; i < K_half; ++i)
			{
				minima[i] = nmax<float>();
				maxima[i] = -nmax<float>();
			}
			dirty_cache = true;
			dirty_center = true;
		}

		glm::vec2 center() const
		{
			if (dirty_center)
			{
				dirty_center = false;
				_center = {};
				for (glm::vec2 p : cache())
					_center += p;
				_center /= (float)K;
			}
			return _center;
		}

	private:
		const std::array<glm::vec2, K>& cache() const
		{
			if (dirty_cache)
			{
				dirty_cache = false;

				math::Polygon2D cloud;
				cloud.reserve(K);

				for (size_t i = 0; i < K_half; ++i)
				{
					cloud.push_back((glm::vec2)uniform_axis(i) * minima[i]);
					cloud.push_back((glm::vec2)uniform_axis(i) * maxima[i]);
				}

				ConvexHull hull = ConvexHull::wrap(cloud);
				for (size_t i = 0; i < K; ++i)
					_cache[i] = hull.points()[i];
			}
			return _cache;
		}

	public:
		static constexpr UnitVector2D uniform_axis(size_t i)
		{
			return UnitVector2D(i * glm::pi<float>() / K_half);
		}

		static KDOP<K_half> wrap(const math::Polygon2D& polygon)
		{
			KDOP<K_half> kdop;
			kdop.fill_invalid();
			for (glm::vec2 point : polygon)
			{
				for (size_t i = 0; i < K_half; ++i)
				{
					float v = uniform_axis(i).dot(point);
					kdop.set_minimum(i, std::min(kdop.get_minimum(i), v));
					kdop.set_maximum(i, std::max(kdop.get_maximum(i), v));
				}
			}
			return kdop;
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
		mutable glm::vec2 _center{};
		mutable bool dirty_cache = true;
		mutable bool dirty_center = true;

	public:
		size_t get_k_half() const { return axes.size(); }
		size_t get_k() const { return 2 * axes.size(); }
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
			dirty_cache = true;
			dirty_center = true;
		}
		float get_minimum(size_t i) const { return minima[i]; }
		float get_maximum(size_t i) const { return maxima[i]; }
		void set_minimum(size_t i, float minimum) { minima[i] = minimum; dirty_cache = true; dirty_center = true; }
		void set_maximum(size_t i, float maximum) { maxima[i] = maximum; dirty_cache = true; dirty_center = true; }
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
		CustomKDOP(const std::vector<UnitVector2D>& axes, const std::vector<float>& minima, const std::vector<float>& maxima)
		{
			if (axes.size() != minima.size() || axes.size() != maxima.size())
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have the same number of minima, maxima, and axes");
			if (axes.size() < 2)
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 4, but k_half provided is: " + std::to_string(axes.size()));

			this->minima = minima;
			this->maxima = maxima;
			this->axes = axes;
			dirty_cache = true;
			dirty_center = true;
		}
		CustomKDOP(std::vector<UnitVector2D>&& axes, std::vector<float>&& minima, std::vector<float>&& maxima)
		{
			if (axes.size() != minima.size() || axes.size() != maxima.size())
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have the same number of minima, maxima, and axes");
			if (axes.size() < 2)
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 4, but k_half provided is: " + std::to_string(axes.size()));

			this->minima = std::move(minima);
			this->maxima = std::move(maxima);
			this->axes = std::move(axes);
			dirty_cache = true;
			dirty_center = true;
		}

		glm::vec2 center() const
		{
			if (dirty_center)
			{
				dirty_center = false;
				_center = {};
				for (glm::vec2 p : cache())
					_center += p;
				_center / (float)get_k();
			}
			return _center;
		}

	private:
		const std::vector<glm::vec2>& cache() const
		{
			if (dirty_cache)
			{
				dirty_cache = false;

				math::Polygon2D cloud;
				cloud.reserve(get_k());

				for (size_t i = 0; i < get_k_half(); ++i)
				{
					cloud.push_back((glm::vec2)axes[i] * minima[i]);
					cloud.push_back((glm::vec2)axes[i] * maxima[i]);
				}

				ConvexHull hull = ConvexHull::wrap(cloud);
				for (size_t i = 0; i < get_k(); ++i)
					_cache[i] = hull.points()[i];
			}
			return _cache;
		}

	public:
		static CustomKDOP wrap(const math::Polygon2D& polygon, const std::vector<UnitVector2D>& axes)
		{
			return wrap(polygon, dupl(axes));
		}

		static CustomKDOP wrap(const math::Polygon2D& polygon, std::vector<UnitVector2D>&& axes)
		{
			size_t k_half = axes.size();
			if (k_half < 2)
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 4, but k_half provided is: " + std::to_string(k_half));
			std::vector<float> minima(k_half, nmax<float>());
			std::vector<float> maxima(k_half, -nmax<float>());

			for (glm::vec2 point : polygon)
			{
				for (size_t i = 0; i < k_half; ++i)
				{
					float v = axes[i].dot(point);
					minima[i] = std::min(minima[i], v);
					maxima[i] = std::max(maxima[i], v);
				}
			}

			return CustomKDOP(std::move(axes), std::move(minima), std::move(maxima));
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
