#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/elements/ConvexHull.h"
#include "physics/collision/elements/Circle.h"
#include "physics/collision/elements/AABB.h"
#include "physics/collision/elements/OBB.h"
#include "physics/collision/elements/KDOP.h"
#include "physics/collision/Tolerance.h"

namespace oly::col2d::sat
{
	namespace internal
	{
		template<typename Shape1, typename Shape2>
		struct OverlapTest
		{
			static_assert(false, "OverlapTest not supported for the provided shape combo.");
		};

		template<typename Shape1, typename Shape2>
		struct FullOverlapTest
		{
			static OverlapResult call(const Shape1& c1, const Shape2& c2)
			{
				return internal::OverlapTest<Shape1, Shape2>::impl(c1, c2) && internal::OverlapTest<Shape2, Shape1>::impl(c2, c1);
			}
		};

		template<typename Shape1, typename Shape2>
		struct CollisionTest
		{
			static_assert(false, "CollisionTest not supported for the provided shape combo.");
		};

		template<typename Shape1, typename Shape2>
		struct FullCollisionTest
		{
			static CollisionResult call(const Shape1& c1, const Shape2& c2)
			{
				CollisionResult info{ .overlap = true, .penetration_depth = nmax<float>() };
				internal::CollisionTest<Shape1, Shape2>::update_collision(c1, c2, info);
				if (!info.overlap)
					return info;
				internal::CollisionTest<Shape2, Shape1>::update_collision(c2, c1, info.invert());
				if (!info.invert().overlap)
					return info;
				return info;
			}
		};
	}

	template<typename Shape1, typename Shape2>
	inline OverlapResult overlaps(const Shape1& c1, const Shape2& c2)
	{
		return internal::FullOverlapTest<Shape1, Shape2>::call(c1, c2);
	}

	template<typename Shape1, typename Shape2>
	inline CollisionResult collides(const Shape1& c1, const Shape2& c2)
	{
		return internal::FullCollisionTest<Shape1, Shape2>::call(c1, c2);
	}

	template<typename Shape1, typename Shape2>
	inline ContactResult contacts(const Shape1& c1, const Shape2& c2)
	{
		CollisionResult collision = collides(c1, c2);
		if (!collision.overlap)
			return { .overlap = false };

		return {
			.overlap = true,
			.active_feature = {
				.position = c1.deepest_point(-collision.unit_impulse),
				.impulse = (glm::vec2)collision.unit_impulse * collision.penetration_depth
			},
			.static_feature = {
				.position = c2.deepest_point(collision.unit_impulse),
				.impulse = -(glm::vec2)collision.unit_impulse * collision.penetration_depth,
			}
		};
	}

	namespace internal
	{
		static float sat(float min1, float max1, float min2, float max2, UnitVector2D& axis)
		{
			if (min1 > max2)
				return max2 - min1;
			if (min2 > max1)
			{
				axis = -axis;
				return max1 - min2;
			}
			if (min1 < min2)
			{
				if (max1 < max2)
				{
					axis = -axis;
					return max1 - min2;
				}
				else
				{
					if (min1 + max1 < min2 + max2)
					{
						axis = -axis;
						return max1 - min2;
					}
					else
						return max2 - min1;
				}
			}
			else
			{
				if (max1 > max2)
					return max2 - min1;
				else
				{
					if (min1 + max1 < min2 + max2)
					{
						axis = -axis;
						return max1 - min2;
					}
					else
						return max2 - min1;
				}
			}
		}

		template<typename Shape1, typename Shape2>
		static float sat(const Shape1& c1, const Shape2& c2, UnitVector2D& axis)
		{
			auto [min1, max1] = c1.projection_interval(axis);
			auto [min2, max2] = c2.projection_interval(axis);
			return sat(min1, max1, min2, max2, axis);
		}

		template<typename Other>
		struct OverlapTest<ConvexHull, Other>
		{
			static OverlapResult impl(const ConvexHull& c, const Other& other)
			{
				for (size_t i = 0; i < c.points().size(); ++i)
				{
					UnitVector2D axis = c.edge_normal(i);
					if (sat(c, other, axis) < 0.0f)
						return false;
				}
				return true;
			}
		};

		template<typename Other>
		struct CollisionTest<ConvexHull, Other>
		{
			static void update_collision(const ConvexHull& c, const Other& other, CollisionResult& info)
			{
				for (size_t i = 0; i < c.points().size(); ++i)
				{
					UnitVector2D axis = c.edge_normal(i);
					float depth = sat(c, other, axis);
					if (depth < 0.0f)
					{
						info.overlap = false;
						info.penetration_depth = 0.0f;
						return;
					}
					else if (depth < info.penetration_depth)
					{
						info.penetration_depth = depth;
						info.unit_impulse = axis;
					}
				}
			}
		};

		template<typename Other>
		struct OverlapTest<AABB, Other>
		{
			static OverlapResult impl(const AABB& c, const Other& other)
			{
				{
					UnitVector2D axis = UnitVector2D::RIGHT;
					auto [min2, max2] = other.projection_interval(axis);
					if (sat(c.x1, c.x2, min2, max2, axis) < 0.0f)
						return false;
				}

				{
					UnitVector2D axis = UnitVector2D::UP;
					auto [min2, max2] = other.projection_interval(axis);
					if (sat(c.y1, c.y2, min2, max2, axis) < 0.0f)
						return false;
				}

				return true;
			}
		};

		template<typename Other>
		struct CollisionTest<AABB, Other>
		{
			static void update_collision(const AABB& c, const Other& other, CollisionResult& info)
			{
				UnitVector2D axis = UnitVector2D::RIGHT;
				float depth = sat(c, other, axis);
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
					return;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = axis;
				}
				axis = UnitVector2D::UP;
				depth = sat(c, other, axis);
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = axis;
				}
			}
		};

		template<typename Other>
		struct OverlapTest<OBB, Other>
		{
			static OverlapResult impl(const OBB& c, const Other& other)
			{
				{
					UnitVector2D axis = c.get_major_axis();
					float cw = axis.dot(c.center);
					auto [min2, max2] = other.projection_interval(axis);
					if (sat(cw - 0.5f * c.width, cw + 0.5f * c.width, min2, max2, axis) < 0.0f)
						return false;
				}

				{
					UnitVector2D axis = c.get_minor_axis();
					float ch = axis.dot(c.center);
					auto [min2, max2]  = other.projection_interval(axis);
					if (sat(ch - 0.5f * c.height, ch + 0.5f * c.height, min2, max2, axis) < 0.0f)
						return false;
				}

				return true;
			}
		};

		template<typename Other>
		struct CollisionTest<OBB, Other>
		{
			static void update_collision(const OBB& c, const Other& other, CollisionResult& info)
			{
				UnitVector2D axis = c.get_major_axis();
				float depth = sat(c, other, axis);
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
					return;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = axis;
				}
				axis = c.get_minor_axis();
				depth = sat(c, other, axis);
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = axis;
				}
			}
		};

		template<size_t K, typename Other>
		struct OverlapTest<KDOP<K>, Other>
		{
			static OverlapResult impl(const KDOP<K>& c, const Other& other)
			{
				for (size_t i = 0; i < K; ++i)
				{
					fpair i2 = other.projection_interval(KDOP<K>::uniform_axis(i));
					if (std::min(c.get_clipped_maximum(i), i2.second) - std::max(c.get_clipped_minimum(i), i2.first) < 0.0f)
						return false;
				}
				return true;
			}
		};

		template<size_t K, typename Other>
		struct CollisionTest<KDOP<K>, Other>
		{
			static void update_collision(const KDOP<K>& c, const Other& other, CollisionResult& info)
			{
				for (size_t i = 0; i < K; ++i)
				{
					UnitVector2D axis = KDOP<K>::uniform_axis(i);
					auto [min2, max2] = other.projection_interval(axis);
					float depth = sat(c.get_clipped_minimum(i), c.get_clipped_maximum(i), min2, max2, axis);
					if (depth < 0.0f)
					{
						info.overlap = false;
						info.penetration_depth = 0.0f;
						return;
					}
					else if (depth < info.penetration_depth)
					{
						info.penetration_depth = depth;
						info.unit_impulse = axis;
					}
				}
			}
		};

		template<size_t K>
		struct FullOverlapTest<KDOP<K>, KDOP<K>>
		{
			static OverlapResult call(const KDOP<K>& c1, const KDOP<K>& c2)
			{
				for (size_t i = 0; i < K; ++i)
				{
					if (std::min(c1.get_clipped_maximum(i), c2.get_clipped_maximum(i)) - std::max(c1.get_clipped_minimum(i), c2.get_clipped_minimum(i)) < 0.0f)
						return false;
				}
				return true;
			}
		};

		template<size_t K>
		struct FullCollisionTest<KDOP<K>, KDOP<K>>
		{
			static CollisionResult call(const KDOP<K>& c1, const KDOP<K>& c2)
			{
				CollisionResult info{ .overlap = true, .penetration_depth = nmax<float>() };
				for (size_t i = 0; i < K; ++i)
				{
					UnitVector2D axis = KDOP<K>::uniform_axis(i);
					float depth = sat(c1.get_clipped_minimum(i), c1.get_clipped_maximum(i), c2.get_clipped_minimum(i), c2.get_clipped_maximum(i), axis);
					if (depth < 0.0f)
						return { .overlap = false };
					else if (depth < info.penetration_depth)
					{
						info.penetration_depth = depth;
						info.unit_impulse = axis;
					}
				}
				return info;
			}
		};
	}
}
