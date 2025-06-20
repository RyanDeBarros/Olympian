#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Errors.h"
#include "core/base/Transforms.h"
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
		template<size_t K>
		struct KDOPGlobalAccess;

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
		friend struct internal::KDOPGlobalAccess<K>;

		std::array<float, K> minima;
		std::array<float, K> maxima;
		mutable math::Polygon2D _cache;
		mutable glm::vec2 _center{};
		mutable bool dirty_cache = true;
		mutable bool dirty_center = true;
		mutable bool dirty_clipped = true;
		mutable std::array<fpair, K> _clipped_extrema;

		glm::mat3x2 global = DEFAULT_3x2;
		glm::mat3x2 ginv = DEFAULT_3x2;

		void flag() const { dirty_cache = true; dirty_center = true; dirty_clipped = true; }

	public:
		float get_minimum(size_t i) const { return minima[i]; }
		float get_maximum(size_t i) const { return maxima[i]; }
		float get_clipped_minimum(size_t i) const { return clipped_extrema()[i].first; }
		float get_clipped_maximum(size_t i) const { return clipped_extrema()[i].second; }
		void set_minimum(size_t i, float minimum) { minima[i] = minimum; flag(); }
		void set_maximum(size_t i, float maximum) { maxima[i] = maximum; flag(); }
		void set_minima(const std::array<float, K>& min) { minima = min; flag(); }
		void set_maxima(const std::array<float, K>& max) { maxima = max; flag(); }

		KDOP() = default;
		KDOP(const std::array<float, K>& minima, const std::array<float, K>& maxima) : minima(minima), maxima(maxima) { flag(); }
		KDOP(const std::array<fpair, K>& extrema)
		{
			for (size_t i = 0; i < K; ++i)
			{
				minima[i] = extrema[i].first;
				maxima[i] = extrema[i].second;
			}
			flag();
		}
		
		void fill_invalid()
		{
			for (size_t i = 0; i < K; ++i)
			{
				minima[i] = nmax<float>();
				maxima[i] = -nmax<float>();
			}
			flag();
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

	private:
		const std::array<fpair, K>& clipped_extrema() const
		{
			if (dirty_clipped)
			{
				dirty_clipped = false;
				for (size_t i = 0; i < K; ++i)
					_clipped_extrema[i] = internal::polygon_projection_interval(cache(), uniform_axis(i));
			}
			return _clipped_extrema;
		}

	public:
		fpair projection_interval(const UnitVector2D& axis) const
		{
			float i = axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
			{
				int j = unsigned_mod(roundi(i), 2 * K);
				if (j < K)
					return { get_clipped_minimum(j), get_clipped_maximum(j) };
				else
					return { -get_clipped_maximum(j - K), -get_clipped_minimum(j - K) };
			}
			else
				return { axis.dot(deepest_point(-axis)), axis.dot(deepest_point(axis)) };
		}

		float projection_min(const UnitVector2D& axis) const
		{
			float i = axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
			{
				int j = unsigned_mod(roundi(i), 2 * K);
				if (j < K)
					return get_clipped_minimum(j);
				else
					return -get_clipped_maximum(j - K);
			}
			else
				return axis.dot(deepest_point(-axis));
		}

		float projection_max(const UnitVector2D& axis) const
		{
			float i = axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
			{
				int j = unsigned_mod(roundi(i), 2 * K);
				if (j < K)
					return get_clipped_maximum(j);
				else
					return -get_clipped_minimum(j - K);
			}
			else
				return axis.dot(deepest_point(axis));
		}

		UnitVector2D edge_normal(size_t i) const
		{
			return uniform_axis(i);
		}
		
		glm::vec2 deepest_point(const UnitVector2D& axis) const
		{
			float i = axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
			{
				int j = unsigned_mod(roundi(i), 2 * K);
				return (j < K ? get_clipped_maximum(j) : get_clipped_minimum(j - K)) * (glm::vec2)uniform_axis(j % K);
			}
			else
			{
				int i1 = unsigned_mod((int)floorf(i), 2 * K);
				glm::vec2 p1 = (i1 < K ? get_clipped_maximum(i1) : get_clipped_minimum(i1 - K)) * (glm::vec2)uniform_axis(i1 % K);
				float m1 = math::mag_sqrd(p1);
				
				int i2 = unsigned_mod((int)ceilf(i), 2 * K);
				glm::vec2 p2 = (i2 < K ? get_clipped_maximum(i2) : get_clipped_minimum(i2 - K)) * (glm::vec2)uniform_axis(i2 % K);
				float m2 = math::mag_sqrd(p2);

				glm::vec2 xpt = math::intersection_by_normals(p1, uniform_axis(i1), p2, uniform_axis(i2));
				float m3 = math::mag_sqrd(xpt);
				
				size_t sel = max_of(m1, m2, m3);
				if (sel == 0)
					return p1;
				else if (sel == 0)
					return p2;
				else
					return xpt;
			}
		}
	};

	namespace internal
	{
		template<size_t K>
		struct KDOPGlobalAccess
		{
			static const glm::mat3x2& get_global(const KDOP<K>& c)
			{
				return c.global;
			}

			static const glm::mat3x2& get_ginv(const KDOP<K>& c)
			{
				return c.ginv;
			}

			static KDOP<K> create_affine_kdop(const KDOP<K>& c, const glm::mat3x2& g)
			{
				KDOP<K> tc = c;
				tc.global = g;
				tc.ginv = glm::inverse(glm::mat3{ glm::vec3(g[0], 0.0f), glm::vec3(g[1], 0.0f), glm::vec3(g[2], 1.0f) });
				return tc;
			}

			static CopyPtr<KDOP<K>> create_affine_kdop_ptr(const KDOP<K>& c, const glm::mat3x2& g)
			{
				CopyPtr<KDOP<K>> tc(c);
				tc->global = g;
				tc->ginv = glm::inverse(glm::mat3{ glm::vec3(g[0], 0.0f), glm::vec3(g[1], 0.0f), glm::vec3(g[2], 1.0f) });
				return tc;
			}

			static bool has_no_global(const KDOP<K>& c)
			{
				return c.global == DEFAULT_3x2;
			}

			static glm::vec2 global_center(const KDOP<K>& c)
			{
				return transform_point(c.global, c.center);
			}
		};
	}
}
