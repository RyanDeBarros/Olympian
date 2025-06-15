#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Errors.h"
#include "core/math/Geometry.h"
#include "core/types/Approximate.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/Common.h"
#include "physics/collision/Tolerance.h"

#include "core/types/CopyPtr.h"

#include <array>
#include <string>
#include <vector>

namespace oly::col2d
{
	namespace internal
	{
		inline math::Polygon2D initial_kdop_polygon(const UnitVector2D& axis0, float min0, float max0, const UnitVector2D& axis1, float min1, float max1)
		{
			if (axis0.near_parallel(axis1, LINEAR_TOLERANCE))
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "First 2 kDOP axes are parallel");

			static const auto intersection = [](const UnitVector2D& axis0, float m0, const UnitVector2D& axis1, float m1) -> glm::vec2 {
				glm::mat2 A = { { axis0.x(), axis1.x() }, { axis0.y(), axis1.y() } };
				return glm::inverse(A) * glm::vec2{ m0, m1 };
				};

			return {
				intersection(axis0, min0, axis1, min1),
				intersection(axis0, max0, axis1, min1),
				intersection(axis0, max0, axis1, max1),
				intersection(axis0, min0, axis1, max1)
			};
		}
	}

	template<size_t K>
	struct KDOP
	{
		static_assert(K >= 2, "kDOP must have degree at least 2.");

	private:
		std::array<float, K> minima;
		std::array<float, K> maxima;
		mutable math::Polygon2D _cache;
		mutable glm::vec2 _center{};
		mutable bool dirty_cache = true;
		mutable bool dirty_center = true;

	public:
		const std::array<float, K>& get_minima() const { return minima; }
		const std::array<float, K>& get_maxima() const { return maxima; }
		void set_minima(const std::array<float, K>& min) { minima = min; dirty_cache = true; dirty_center = true; }
		void set_maxima(const std::array<float, K>& max) { maxima = max; dirty_cache = true; dirty_center = true; }
		float get_minimum(size_t i) const { return minima[i]; }
		float get_maximum(size_t i) const { return maxima[i]; }
		void set_minimum(size_t i, float minimum) { minima[i] = minimum; dirty_cache = true; dirty_center = true; }
		void set_maximum(size_t i, float maximum) { maxima[i] = maximum; dirty_cache = true; dirty_center = true; }
		
		KDOP() = default;
		KDOP(const std::array<float, K>& minima, const std::array<float, K>& maxima) : minima(minima), maxima(maxima) { dirty_cache = true; dirty_center = true; }
		KDOP(const std::array<std::pair<float, float>, K>& extrema)
		{
			for (size_t i = 0; i < K; ++i)
			{
				minima[i] = extrema[i].first;
				maxima[i] = extrema[i].second;
			}
			dirty_cache = true;
			dirty_center = true;
		}
		
		void fill_invalid()
		{
			for (size_t i = 0; i < K; ++i)
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
		const math::Polygon2D& cache() const
		{
			if (dirty_cache)
			{
				dirty_cache = false;
				_cache = internal::initial_kdop_polygon(uniform_axis(0), minima[0], maxima[0], uniform_axis(1), minima[1], maxima[1]);
				for (size_t i = 2; i < K; ++i)
				{
					UnitVector2D axis = uniform_axis(i);
					_cache = math::clip_polygon(_cache, -axis, -minima[i]);
					_cache = math::clip_polygon(_cache, axis, maxima[i]);
					if (_cache.empty())
						break;
				}
			}
			return _cache;
		}

	public:
		static constexpr UnitVector2D uniform_axis(size_t i)
		{
			return UnitVector2D((float)i * glm::pi<float>() / K);
		}

		static KDOP<K> wrap(const math::Polygon2D& polygon)
		{
			KDOP<K> kdop;
			kdop.fill_invalid();
			for (glm::vec2 point : polygon)
			{
				for (size_t i = 0; i < K; ++i)
				{
					float v = uniform_axis(i).dot(point);
					kdop.set_minimum(i, std::min(kdop.get_minimum(i), v));
					kdop.set_maximum(i, std::max(kdop.get_maximum(i), v));
				}
			}
			return kdop;
		}

		static CopyPtr<KDOP<K>> wrap_copy_ptr(const math::Polygon2D& polygon)
		{
			CopyPtr<KDOP<K>> kdop = make_copy_ptr<KDOP<K>>();
			kdop->fill_invalid();
			for (glm::vec2 point : polygon)
			{
				for (size_t i = 0; i < K; ++i)
				{
					float v = uniform_axis(i).dot(point);
					kdop->set_minimum(i, std::min(kdop.get_minimum(i), v));
					kdop->set_maximum(i, std::max(kdop.get_maximum(i), v));
				}
			}
			return kdop;
		}

		const math::Polygon2D& points() const
		{
			return cache();
		}

		std::pair<float, float> projection_interval(const UnitVector2D& axis) const
		{
			for (size_t i = 0; i < K; ++i)
			{
				UnitVector2D::ParallelState p = axis.near_parallel_state(uniform_axis(i), LINEAR_TOLERANCE);
				if (p == UnitVector2D::ParallelState::SAME_DIRECTION)
					return { minima[i], maxima[i] };
				else if (p == UnitVector2D::ParallelState::OPPOSITE_DIRECTION)
					return { -maxima[i], -minima[i] };
			}
			return internal::polygon_projection_interval(cache(), axis);
		}

		float projection_min(const UnitVector2D& axis) const
		{
			for (size_t i = 0; i < K; ++i)
			{
				UnitVector2D::ParallelState p = axis.near_parallel_state(uniform_axis(i), LINEAR_TOLERANCE);
				if (p == UnitVector2D::ParallelState::SAME_DIRECTION)
					return minima[i];
				else if (p == UnitVector2D::ParallelState::OPPOSITE_DIRECTION)
					return -maxima[i];
			}
			return internal::polygon_projection_min(cache(), axis);
		}

		float projection_max(const UnitVector2D& axis) const
		{
			for (size_t i = 0; i < K; ++i)
			{
				UnitVector2D::ParallelState p = axis.near_parallel_state(uniform_axis(i), LINEAR_TOLERANCE);
				if (p == UnitVector2D::ParallelState::SAME_DIRECTION)
					return maxima[i];
				else if (p == UnitVector2D::ParallelState::OPPOSITE_DIRECTION)
					return -minima[i];
			}
			return internal::polygon_projection_max(cache(), axis);
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
		size_t get_k() const { return axes.size(); }
		const std::vector<UnitVector2D>& get_axes() const { return axes; }
		const std::vector<float>& get_minima() const { return minima; }
		const std::vector<float>& get_maxima() const { return maxima; }
		void set_k(size_t k)
		{
			if (k < 2)
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 2, but the degree provided is: " + std::to_string(k));
			axes.resize(k);
			minima.resize(k);
			maxima.resize(k);
			dirty_cache = true;
			dirty_center = true;
		}
		float get_minimum(size_t i) const { return minima[i]; }
		float get_maximum(size_t i) const { return maxima[i]; }
		void set_minimum(size_t i, float minimum) { minima[i] = minimum; dirty_cache = true; dirty_center = true; }
		void set_maximum(size_t i, float maximum) { maxima[i] = maximum; dirty_cache = true; dirty_center = true; }
		void set_axes(const std::vector<UnitVector2D>& axes)
		{
			set_k(axes.size());
			for (size_t i = 0; i < get_k(); ++i)
				this->axes[i] = axes[i];
		}
		void set_extrema(const std::vector<std::pair<float, float>>& extrema)
		{
			set_k(extrema.size());
			for (size_t i = 0; i < get_k(); ++i)
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
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 2, but the degree provided is: " + std::to_string(axes.size()));

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
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 2, but the degree provided is: " + std::to_string(axes.size()));

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
				_cache = internal::initial_kdop_polygon(edge_normal(0), minima[0], maxima[0], edge_normal(1), minima[1], maxima[1]);
				for (size_t i = 2; i < get_k(); ++i)
				{
					UnitVector2D axis = edge_normal(i);
					_cache = math::clip_polygon(_cache, -axis, -minima[i]);
					_cache = math::clip_polygon(_cache, axis, maxima[i]);
					if (_cache.empty())
						break;
				}
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
			const size_t k = axes.size();
			if (k < 2)
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 2, but the degree provided is: " + std::to_string(k));
			std::vector<float> minima(k, nmax<float>());
			std::vector<float> maxima(k, -nmax<float>());

			for (glm::vec2 point : polygon)
			{
				for (size_t i = 0; i < k; ++i)
				{
					float v = axes[i].dot(point);
					minima[i] = std::min(minima[i], v);
					maxima[i] = std::max(maxima[i], v);
				}
			}

			return CustomKDOP(std::move(axes), std::move(minima), std::move(maxima));
		}

		static CopyPtr<CustomKDOP> wrap_copy_ptr(const math::Polygon2D& polygon, const std::vector<UnitVector2D>& axes)
		{
			return wrap_copy_ptr(polygon, dupl(axes));
		}

		static CopyPtr<CustomKDOP> wrap_copy_ptr(const math::Polygon2D& polygon, std::vector<UnitVector2D>&& axes)
		{
			const size_t k = axes.size();
			if (k < 2)
				throw Error(ErrorCode::BAD_COLLISION_SHAPE, "kDOP must have degree at least 2, but the degree provided is: " + std::to_string(k));
			std::vector<float> minima(k, nmax<float>());
			std::vector<float> maxima(k, -nmax<float>());

			for (glm::vec2 point : polygon)
			{
				for (size_t i = 0; i < k; ++i)
				{
					float v = axes[i].dot(point);
					minima[i] = std::min(minima[i], v);
					maxima[i] = std::max(maxima[i], v);
				}
			}

			return make_copy_ptr<CustomKDOP>(std::move(axes), std::move(minima), std::move(maxima));
		}

		const math::Polygon2D& points() const
		{
			return cache();
		}

		std::pair<float, float> projection_interval(const UnitVector2D& axis) const
		{
			for (size_t i = 0; i < get_k(); ++i)
			{
				UnitVector2D::ParallelState p = axis.near_parallel_state(axes[i], LINEAR_TOLERANCE);
				if (p == UnitVector2D::ParallelState::SAME_DIRECTION)
					return { minima[i], maxima[i] };
				else if (p == UnitVector2D::ParallelState::OPPOSITE_DIRECTION)
					return { -maxima[i], -minima[i] };
			}
			return internal::polygon_projection_interval(cache(), axis);
		}

		float projection_min(const UnitVector2D& axis) const
		{
			for (size_t i = 0; i < get_k(); ++i)
			{
				UnitVector2D::ParallelState p = axis.near_parallel_state(axes[i], LINEAR_TOLERANCE);
				if (p == UnitVector2D::ParallelState::SAME_DIRECTION)
					return minima[i];
				else if (p == UnitVector2D::ParallelState::OPPOSITE_DIRECTION)
					return -maxima[i];
			}
			return internal::polygon_projection_min(cache(), axis);
		}

		float projection_max(const UnitVector2D& axis) const
		{
			for (size_t i = 0; i < get_k(); ++i)
			{
				UnitVector2D::ParallelState p = axis.near_parallel_state(axes[i], LINEAR_TOLERANCE);
				if (p == UnitVector2D::ParallelState::SAME_DIRECTION)
					return maxima[i];
				else if (p == UnitVector2D::ParallelState::OPPOSITE_DIRECTION)
					return -minima[i];
			}
			return internal::polygon_projection_max(cache(), axis);
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
