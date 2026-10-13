#pragma once

#include "core/base/Approximate.h"
#include "core/base/Errors.h"
#include "core/base/SimpleMath.h"
#include "core/base/Transforms.h"
#include "core/base/UnitVector.h"

#include "core/math/Geometry.h"
#include "core/math/Triangulation.h"

#include "physics/collision/Tolerance.h"

#include "physics/collision/methods/CollisionInfo.h"

#include "physics/collision/elements/Common.h"
#include "physics/collision/elements/ConvexHull.h"

#include <array>
#include <string>
#include <vector>

namespace oly::col2d
{
	namespace internal
	{
		template<size_t K>
		struct KDOPGlobalAccess;

        extern math::Polygon2D initial_kdop_polygon(const UnitVector2D& axis0, float min0, float max0, const UnitVector2D& axis1, float min1, float max1);
	}

	template<size_t K>
	struct KDOP
	{
		static_assert(K >= 2, "kDOP must have degree at least 2.");
		static constexpr float kPiOverK = glm::pi<float>() / K;

	private:
		friend struct internal::KDOPGlobalAccess<K>;

		std::array<float, K> minima_;
		std::array<float, K> maxima_;
		mutable math::Polygon2D cache_;
		mutable glm::vec2 center_{};
		mutable bool dirty_cache_ = true;
		mutable bool dirty_center_ = true;
		mutable bool dirty_clipped_ = true;
		mutable std::array<fpair, K> clipped_extrema_;

		glm::mat2 global_ = 1.0f;
		glm::vec2 global_offset_ = {};
		glm::mat2 global_inverse_ = 1.0f;

	public:
		KDOP()
        {
            minima_.fill(0.0f);
            maxima_.fill(0.0f);
        }

		KDOP(const std::array<float, K>& minima, const std::array<float, K>& maxima)
            : minima_(minima)
            , maxima_(maxima)
        {
            flag();
        }
		
        explicit KDOP(const std::array<fpair, K>& extrema)
		{
			for (size_t i = 0; i < K; ++i)
			{
				minima_[i] = extrema[i].first;
				maxima_[i] = extrema[i].second;
			}

			flag();
		}

    public:
        static constexpr UnitVector2D uniform_axis(size_t i)
        {
            return UnitVector2D((float)i * kPiOverK);
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

        float get_minimum(size_t i) const
        {
            return minima_[i];
        }
        
        float get_maximum(size_t i) const
        {
            return maxima_[i];
        }

        float get_clipped_minimum(size_t i) const
        {
            return clipped_extrema()[i].first;
        }
        
        float get_clipped_maximum(size_t i) const
        {
            return clipped_extrema()[i].second;
        }

        void set_minimum(size_t i, float minimum)
        {
            minima_[i] = minimum;
            flag();
        }

        void set_maximum(size_t i, float maximum)
        {
            maxima_[i] = maximum;
            flag();
        }

        void set_minima(const std::array<float, K>& min)
        {
            minima_ = min;
            flag();
        }

        void set_maxima(const std::array<float, K>& max)
        {
            maxima_ = max;
            flag();
        }

		void fill_invalid()
		{
			for (size_t i = 0; i < K; ++i)
			{
				minima_[i] = nmax<float>();
				maxima_[i] = -nmax<float>();
			}

            flag();
		}

		glm::vec2 center() const
		{
			if (dirty_center_)
			{
				dirty_center_ = false;
				center_ = {};
				for (glm::vec2 p : cache())
					center_ += p;
				center_ /= (float)K;
			}
			return center_;
		}

		const math::Polygon2D& points() const
		{
			return cache();
		}

		float global_clipped_minimum(size_t i) const
		{
			return i < K ? global_extremum(uniform_axis(i), get_clipped_minimum(i)) : global_extremum(-uniform_axis(i - K), -get_clipped_maximum(i - K));
		}

		float global_clipped_maximum(size_t i) const
		{
			return i < K ? global_extremum(uniform_axis(i), get_clipped_maximum(i)) : global_extremum(-uniform_axis(i - K), -get_clipped_minimum(i - K));
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
				return { global_extremum(local_axis, local_axis.dot(local_deepest_manifold(-local_axis).pt())),
					global_extremum(local_axis, local_axis.dot(local_deepest_manifold(local_axis).pt())) };
		}

		float projection_min(const UnitVector2D& axis) const
		{
			UnitVector2D local_axis = get_local_axis(axis);
			float i = local_axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
				return global_clipped_minimum(unsigned_mod(roundi(i), 2 * K));
			else
				return global_extremum(local_axis, local_axis.dot(local_deepest_manifold(-local_axis).pt()));
		}

		float projection_max(const UnitVector2D& axis) const
		{
			UnitVector2D local_axis = get_local_axis(axis);
			float i = local_axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
				return global_clipped_maximum(unsigned_mod(roundi(i), 2 * K));
			else
				return global_extremum(local_axis, local_axis.dot(local_deepest_manifold(local_axis).pt()));
		}

		UnitVector2D edge_normal(size_t i) const
		{
			return UnitVector2D(glm::transpose(global_inverse_) * uniform_axis(i));
		}

		ContactManifold deepest_manifold(const UnitVector2D& axis) const
		{
			ContactManifold local_manifold = local_deepest_manifold(get_local_axis(axis));
			ContactManifold global_manifold;
			global_manifold.p1 = global_ * local_manifold.p1 + global_offset_;
			if (!local_manifold.Single)
			{
				global_manifold.Single = false;
				global_manifold.p2 = global_ * local_manifold.p2 + global_offset_;
			}
			return global_manifold;
		}
		
	private:
        void flag() const
        {
            dirty_cache_ = true;
            dirty_center_ = true;
            dirty_clipped_ = true;
        }

        const math::Polygon2D& cache() const
        {
            if (dirty_cache_)
            {
                dirty_cache_ = false;
                cache_ = internal::initial_kdop_polygon(uniform_axis(0), minima_[0], maxima_[0], uniform_axis(1), minima_[1], maxima_[1]);
                if constexpr (K > 2)
                {
                    for (size_t i = 2; i < K; ++i)
                    {
                        UnitVector2D axis = uniform_axis(i);
                        cache_ = math::clip_polygon(cache_, -axis, -minima_[i]);
                        cache_ = math::clip_polygon(cache_, axis, maxima_[i]);
                        if (cache_.empty())
                            break;
                    }
                }
                math::simplify(cache_, kLinearTolerance);
                for (size_t i = 0; i < cache_.size(); ++i)
                    cache_[i] = global_ * cache_[i] + global_offset_;
            }
            return cache_;
        }

        const std::array<fpair, K>& clipped_extrema() const
        {
            if (dirty_clipped_)
            {
                dirty_clipped_ = false;
                for (size_t i = 0; i < K; ++i)
                {
                    fpair global_clipped_extrema = internal::polygon_projection_interval(cache(), edge_normal(i));
                    clipped_extrema_[i] = { local_extremum(edge_normal(i), global_clipped_extrema.first), local_extremum(edge_normal(i), global_clipped_extrema.second) };
                }
            }
            return clipped_extrema_;
        }

        UnitVector2D get_local_axis(UnitVector2D axis) const
        {
            return glm::transpose(global_) * axis;
        }

        float local_extremum(UnitVector2D axis, float extremum) const
        {
            return (extremum - axis.dot(global_offset_)) * math::inv_magnitude(glm::transpose(global_) * (glm::vec2)axis);
        }

        float global_extremum(UnitVector2D axis, float extremum) const
        {
            return (extremum + axis.dot(global_inverse_ * global_offset_)) * math::inv_magnitude(glm::transpose(global_inverse_) * (glm::vec2)axis);
        }

		glm::vec2 corner_intersection(int i1, int i2) const
		{
			glm::vec2 p1 = (i1 < K ? get_clipped_maximum(i1) : get_clipped_minimum(i1 - K)) * (glm::vec2)uniform_axis(i1 % K);
			glm::vec2 p2 = (i2 < K ? get_clipped_maximum(i2) : get_clipped_minimum(i2 - K)) * (glm::vec2)uniform_axis(i2 % K);
			return math::intersection_by_normals(p1, uniform_axis(i1), p2, uniform_axis(i2));
		}

		ContactManifold local_deepest_manifold(const UnitVector2D& axis) const
		{
			float i = axis.rotation() * K * glm::one_over_pi<float>();
			if (near_multiple(i, 1.0f))
			{
				ContactManifold manifold;
				manifold.Single = false;
				int j = unsigned_mod(roundi(i), 2 * K);
				manifold.p1 = corner_intersection(j, unsigned_mod(j + 1, 2 * K));
				manifold.p2 = corner_intersection(unsigned_mod(j - 1, 2 * K), j);
				return manifold;
			}
			else
			{
				int i1 = unsigned_mod((int)floorf(i), 2 * K);
				int i2 = unsigned_mod((int)ceilf(i), 2 * K);
				return { .p1 = corner_intersection(i1, i2) };
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
				return c.global_;
			}

			static glm::vec2 get_global_offset(const KDOP<K>& c)
			{
				return c.global_offset_;
			}

			static KDOP<K> create_affine_kdop(const KDOP<K>& c, const glm::mat3x2& g)
			{
				KDOP<K> tc = c;
				glm::mat3x2 full = augment(g) * augment(c.global_, c.global_offset_);
				tc.global_ = glm::mat2(full);
				tc.global_offset_ = full[2];
				tc.global_inverse_ = glm::inverse(tc.global_);
				tc.flag();
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

				float m10 = math::mag_sqrd(c1.global_[0]);
				float m11 = math::mag_sqrd(c1.global_[1]);
				float m20 = math::mag_sqrd(c2.global_[0]);
				float m21 = math::mag_sqrd(c2.global_[1]);

				if (approx(m10, m11) && approx(m20, m21)) // uniform scaling
				{
					UnitVector2D v10(c1.global_[0]);
					UnitVector2D v11(c1.global_[1]);
					UnitVector2D v20(c2.global_[0]);
					UnitVector2D v21(c2.global_[1]);

					float rotation_offset = v20.rotation() - v10.rotation();
					if (!approx(rotation_offset, v21.rotation() - v11.rotation()) || !near_multiple(rotation_offset, KDOP<K>::kPiOverK))
						return false;

					v20.rotate(-rotation_offset);
					v21.rotate(-rotation_offset);
					
					if (nonrotated_compatible_globals(v10, v11, v20, v21, offset2, sign2))
					{
						offset2 += roundi(rotation_offset / KDOP<K>::kPiOverK);
						return true;
					}
					return false;
				}
				else if (approx(m10 * m21, m20 * m11)) // proportional scaling
				{
					UnitVector2D v10(c1.global_[0]);
					UnitVector2D v11(c1.global_[1]);
					UnitVector2D v20(c2.global_[0]);
					UnitVector2D v21(c2.global_[1]);
					return nonrotated_compatible_globals(v10, v11, v20, v21, offset2, sign2);
				}
				else
					return false;
			}

			static glm::vec2 global_center(const KDOP<K>& c)
			{
				return c.global_ * c.center() + c.global_offset_;
			}

			static glm::vec2 local_point(const KDOP<K>& c, glm::vec2 v)
			{
				return c.global_inverse_ * (v - c.global_offset_);
			}

			static glm::vec2 local_direction(const KDOP<K>& c, glm::vec2 v)
			{
				return c.global_inverse_ * v;
			}

			static glm::vec2 local_normal(const KDOP<K>& c, glm::vec2 n)
			{
				return glm::transpose(c.global_) * n;
			}

			static Ray local_ray(const KDOP<K>& c, Ray ray)
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
				return c.global_ * v + c.global_offset_;
			}

			static glm::vec2 global_direction(const KDOP<K>& c, glm::vec2 v)
			{
				return c.global_ * v;
			}

			static glm::vec2 global_normal(const KDOP<K>& c, glm::vec2 n)
			{
				return glm::transpose(c.global_inverse_) * n;
			}
		};
	}
}
