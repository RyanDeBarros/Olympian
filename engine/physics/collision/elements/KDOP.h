#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Errors.h"
#include "core/base/Transforms.h"
#include "core/math/Geometry.h"
#include "core/math/Triangulation.h"
#include "core/types/Approximate.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/Common.h"
#include "physics/collision/Tolerance.h"

#include "core/containers/CopyPtr.h"

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
		static constexpr float PI_OVER_K = glm::pi<float>() / K;

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

		glm::mat2 global = 1.0f;
		glm::vec2 global_offset = {};
		glm::mat2 global_inverse = 1.0f;

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
		explicit KDOP(const std::array<fpair, K>& extrema)
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
				if constexpr (K > 2)
				{
					for (size_t i = 2; i < K; ++i)
					{
						UnitVector2D axis = uniform_axis(i);
						_cache = math::clip_polygon(_cache, -axis, -minima[i]);
						_cache = math::clip_polygon(_cache, axis, maxima[i]);
						if (_cache.empty())
							break;
					}
				}
				math::simplify(_cache, LINEAR_TOLERANCE);
				for (size_t i = 0; i < _cache.size(); ++i)
					_cache[i] = global * _cache[i] + global_offset;
			}
			return _cache;
		}

	public:
		static constexpr UnitVector2D uniform_axis(size_t i)
		{
			return UnitVector2D((float)i * PI_OVER_K);
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
				{
					fpair global_clipped_extrema = internal::polygon_projection_interval(cache(), edge_normal(i));
					_clipped_extrema[i] = { local_extremum(edge_normal(i), global_clipped_extrema.first), local_extremum(edge_normal(i), global_clipped_extrema.second) };
				}
			}
			return _clipped_extrema;
		}

		UnitVector2D get_local_axis(UnitVector2D axis) const
		{
			return glm::transpose(global) * axis;
		}

		float local_extremum(UnitVector2D axis, float extremum) const
		{
			return (extremum - axis.dot(global_offset)) * math::inv_magnitude(glm::transpose(global) * (glm::vec2)axis);
		}

		float global_extremum(UnitVector2D axis, float extremum) const
		{
			return (extremum + axis.dot(global_inverse * global_offset)) * math::inv_magnitude(glm::transpose(global_inverse) * (glm::vec2)axis);
		}

	public:
		float global_clipped_minimum(size_t i) const
		{
			return i < K ? global_extremum(uniform_axis(i), get_clipped_minimum(i)) : global_extremum(uniform_axis(i - K), -get_clipped_maximum(i - K));
		}

		float global_clipped_maximum(size_t i) const
		{
			return i < K ? global_extremum(uniform_axis(i), get_clipped_maximum(i)) : global_extremum(uniform_axis(i - K), -get_clipped_minimum(i - K));
		}

		fpair projection_interval(const UnitVector2D& axis) const
		{
			UnitVector2D local_axis = get_local_axis(axis);
			float i = local_axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
			{
				int j = unsigned_mod(roundi(i), 2 * K);
				return { global_clipped_minimum(j), global_clipped_maximum(j) };
			}
			else
				return { global_extremum(local_axis, local_axis.dot(local_deepest_point(-local_axis))), global_extremum(local_axis, local_axis.dot(local_deepest_point(local_axis))) };
		}

		float projection_min(const UnitVector2D& axis) const
		{
			UnitVector2D local_axis = get_local_axis(axis);
			float i = local_axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
				return global_clipped_minimum(unsigned_mod(roundi(i), 2 * K));
			else
				return global_extremum(local_axis, local_axis.dot(local_deepest_point(-local_axis)));
		}

		float projection_max(const UnitVector2D& axis) const
		{
			UnitVector2D local_axis = get_local_axis(axis);
			float i = local_axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
				return global_clipped_maximum(unsigned_mod(roundi(i), 2 * K));
			else
				return global_extremum(local_axis, local_axis.dot(local_deepest_point(local_axis)));
		}

		UnitVector2D edge_normal(size_t i) const
		{
			return UnitVector2D(glm::transpose(global_inverse) * uniform_axis(i));
		}

		glm::vec2 deepest_point(const UnitVector2D& axis) const
		{
			return global * local_deepest_point(get_local_axis(axis)) + global_offset;
		}
		
	private:
		glm::vec2 local_deepest_point(const UnitVector2D& axis) const
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
			static const glm::mat2& get_global(const KDOP<K>& c)
			{
				return c.global;
			}

			static glm::vec2 get_global_offset(const KDOP<K>& c)
			{
				return c.global_offset;
			}

			static KDOP<K> create_affine_kdop(const KDOP<K>& c, const glm::mat3x2& g)
			{
				KDOP<K> tc = c;
				glm::mat3x2 full = augment(g) * augment(c.global, c.global_offset);
				tc.global = glm::mat2(full);
				tc.global_offset = full[2];
				tc.global_inverse = glm::inverse(tc.global);
				tc.flag();
				return tc;
			}

			static CopyPtr<KDOP<K>> create_affine_kdop_ptr(const KDOP<K>& c, const glm::mat3x2& g)
			{
				CopyPtr<KDOP<K>> tc(c);
				glm::mat3x2 full = augment(g) * augment(c.global, c.global_offset);
				tc->global = glm::mat2(full);
				tc->global_offset = full[2];
				tc->global_inverse = glm::inverse(tc->global);
				tc->flag();
				return tc;
			}

			static bool compatible_globals(const KDOP<K>& c1, const KDOP<K>& c2, int& offset2, int& sign2)
			{
				static const auto nonrotated_compatible_globals = [](UnitVector2D v10, UnitVector2D v11, UnitVector2D v20, UnitVector2D v21, int& offset2, int& sign2) -> bool {
					if (approx(v10.x(), v20.x()) && approx(v11.x(), v21.x()))
					{
						if (approx(v10.y(), v20.y()) && approx(v11.y(), v21.y()))
						{
							offset2 = 0;
							sign2 = 1;
							return true;
						}
						else if (approx(v10.y(), -v20.y()) && approx(v11.y(), -v21.y()))
						{
							offset2 = 0;
							sign2 = -1;
							return true;
						}
					}
					else if (approx(v10.x(), -v20.x()) && approx(v11.x(), -v21.x()))
					{
						if (approx(v10.y(), v20.y()) && approx(v11.y(), v21.y()))
						{
							offset2 = K;
							sign2 = -1;
							return true;
						}
						else if (approx(v10.y(), -v20.y()) && approx(v11.y(), -v21.y()))
						{
							offset2 = K;
							sign2 = 1;
							return true;
						}
					}
					return false;
					};

				float m10 = math::mag_sqrd(c1.global[0]);
				float m11 = math::mag_sqrd(c1.global[1]);
				float m20 = math::mag_sqrd(c2.global[0]);
				float m21 = math::mag_sqrd(c2.global[1]);

				if (approx(m10, m11) && approx(m20, m21)) // uniform scaling
				{
					UnitVector2D v10(c1.global[0]);
					UnitVector2D v11(c1.global[1]);
					UnitVector2D v20(c2.global[0]);
					UnitVector2D v21(c2.global[1]);

					float rotation_offset = v20.rotation() - v10.rotation();
					if (!approx(rotation_offset, v21.rotation() - v11.rotation()) || !near_multiple(rotation_offset, KDOP<K>::PI_OVER_K))
						return false;

					v20.rotate(-rotation_offset);
					v21.rotate(-rotation_offset);
					
					if (nonrotated_compatible_globals(v10, v11, v20, v21, offset2, sign2))
					{
						offset2 += roundi(rotation_offset / KDOP<K>::PI_OVER_K);
						return true;
					}
					return false;
				}
				else if (approx(m10 * m21, m20 * m11)) // proportional scaling
				{
					UnitVector2D v10(c1.global[0]);
					UnitVector2D v11(c1.global[1]);
					UnitVector2D v20(c2.global[0]);
					UnitVector2D v21(c2.global[1]);
					return nonrotated_compatible_globals(v10, v11, v20, v21, offset2, sign2);
				}
				else
					return false;
			}

			static glm::vec2 global_center(const KDOP<K>& c)
			{
				return c.global * c.center() + c.global_offset;
			}

			static glm::vec2 local_point(const KDOP<K>& c, glm::vec2 v)
			{
				return c.global_inverse * (v - c.global_offset);
			}

			static glm::vec2 local_direction(const KDOP<K>& c, glm::vec2 v)
			{
				return c.global_inverse * v;
			}

			static glm::vec2 local_normal(const KDOP<K>& c, glm::vec2 n)
			{
				return glm::transpose(c.global) * n;
			}

			static Ray local_ray(const KDOP<K>& c, const Ray& ray)
			{
				Ray local_ray = { .origin = local_point(c, ray.origin) };
				if (ray.clip == 0.0f)
					local_ray.direction = local_direction(c, ray.direction);
				else
				{
					glm::vec2 clip = ray.clip * (glm::vec2)ray.direction;
					clip = local_direction(c, clip);
					local_ray.direction = UnitVector2D(clip);
					local_ray.clip = glm::length(clip);
				}
				return local_ray;
			}

			static glm::vec2 global_point(const KDOP<K>& c, glm::vec2 v)
			{
				return c.global * v + c.global_offset;
			}

			static glm::vec2 global_direction(const KDOP<K>& c, glm::vec2 v)
			{
				return c.global * v;
			}

			static glm::vec2 global_normal(const KDOP<K>& c, glm::vec2 n)
			{
				return glm::transpose(c.global_inverse) * n;
			}
		};
	}
}
