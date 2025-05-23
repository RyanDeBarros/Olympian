#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"
#include "physics/collision/abstract/primitives/ConvexHull.h"
#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/primitives/AABB.h"
#include "physics/collision/abstract/primitives/OBB.h"

#include "core/types/Approximate.h"

namespace oly::acm2d::sat
{
	namespace internal
	{
		template<typename Shape1, typename Shape2>
		struct OverlapTest
		{
			static_assert(false, "OverlapTest not supported for the provided shape combo.");
		};

		template<typename Shape1, typename Shape2>
		struct CollisionTest
		{
			static_assert(false, "CollisionTest not supported for the provided shape combo.");
		};
	}

	template<typename Shape1, typename Shape2>
	inline OverlapResult overlaps(const Shape1& c1, const Shape2& c2)
	{
		return internal::OverlapTest<Shape1, Shape2>::impl(c1, c2) && internal::OverlapTest<Shape2, Shape1>::impl(c2, c1);
	}

	template<typename Shape1, typename Shape2>
	inline CollisionResult collides(const Shape1& c1, const Shape2& c2)
	{
		CollisionResult info{ .overlap = true, .penetration_depth = FLT_MAX };
		internal::CollisionTest<Shape1, Shape2>::update_collision(c1, c2, info);
		if (!info.overlap)
			return info;
		internal::CollisionTest<Shape2, Shape1>::update_collision(c2, c1, info);
		if (!info.overlap)
			return info;
		return info;
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
				.position = c1.deepest_point(collision.unit_impulse.get_flipped()),
				.impulse = (glm::vec2)collision.unit_impulse * collision.penetration_depth
			},
			.static_feature = {
				.position = c2.deepest_point(collision.unit_impulse),
				.impulse = (glm::vec2)collision.unit_impulse.get_flipped() * collision.penetration_depth,
			}
		};
	}

	namespace internal
	{
		template<typename Shape1, typename Shape2>
		static float sat(const Shape1& c1, const Shape2& c2, UnitVector2D& axis)
		{
			std::pair<float, float> i1 = c1.projection_interval(axis);
			std::pair<float, float> i2 = c2.projection_interval(axis);
			if (i1.first > i2.first)
				axis.flip();
			return std::min(i1.second, i2.second) - std::max(i1.first, i2.first);
		}

		template<typename Other>
		struct OverlapTest<ConvexHull, Other>
		{
			static OverlapResult impl(const ConvexHull& c, const Other& other)
			{
				for (size_t i = 0; i < c.points.size(); ++i)
					if (sat(c, other, c.edge_normal(i)) < 0.0f)
						return false;
				return true;
			}
		};

		template<typename Other>
		struct CollisionTest<ConvexHull, Other>
		{
			static void update_collision(const ConvexHull& c, const Other& other, CollisionResult& info)
			{
				for (size_t i = 0; i < c.points.size(); ++i)
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
		static void circle_update_collision(const Circle& c, const Other& other, CollisionResult& info, glm::vec2 closest_point)
		{
			if (approx(closest_point, c.center))
				return;
			UnitVector2D axis(closest_point - c.center);
			float depth = sat(c, other, axis);
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
		template<typename Other>
		struct OverlapTest<Circle, Other>
		{
			static OverlapResult impl(const Circle& c, const Other& other)
			{
				glm::vec2 closest_point = c.closest_point(other.points());
				UnitVector2D axis(closest_point - c.center);
				return approx(closest_point, c.center) || sat(c, other, axis) >= 0.0f;
			}
		};

		template<typename Other>
		struct CollisionTest<Circle, Other>
		{
			static void update_collision(const Circle& c, const Other& other, CollisionResult& info)
			{
				circle_update_collision(c, other, info, c.closest_point(other.points()));
			}
		};

		template<>
		struct OverlapTest<Circle, ConvexHull>
		{
			static OverlapResult impl(const Circle& c, const ConvexHull& other)
			{
				glm::vec2 closest_point = c.closest_point(other.points);
				UnitVector2D axis(closest_point - c.center);
				return approx(closest_point, c.center) || sat(c, other, axis) >= 0.0f;
			}
		};

		template<>
		struct CollisionTest<Circle, ConvexHull>
		{
			static void update_collision(const Circle& c, const ConvexHull& other, CollisionResult& info)
			{
				circle_update_collision(c, other, info, c.closest_point(other.points));
			}
		};

		template<typename Other>
		struct OverlapTest<AABB, Other>
		{
			static OverlapResult impl(const AABB& c, const Other& other)
			{
				std::pair<float, float> i = other.projection_interval(UnitVector2D::RIGHT);
				if (std::min(c.x2, i.second) - std::max(c.x1, i.first) < 0.0f)
					return false;

				i = other.projection_interval(UnitVector2D::UP);
				if (std::min(c.y2, i.second) - std::max(c.y1, i.first) < 0.0f)
					return false;

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
				std::array<UnitVector2D, 2> axes = c.get_axes();

				float cw = glm::dot(c.center, (glm::vec2)axes[0]);
				std::pair<float, float> i1 = { cw - 0.5f * c.width, cw + 0.5f * c.width };
				std::pair<float, float> i2 = other.projection_interval(axes[0]);
				if (std::min(i1.second, i2.second) - std::max(i1.first, i2.first) < 0.0f)
					return false;

				float ch = glm::dot(c.center, (glm::vec2)axes[1]);
				i1 = { ch - 0.5f * c.height, ch + 0.5f * c.height };
				i2 = other.projection_interval(axes[1]);
				if (std::min(i1.second, i2.second) - std::max(i1.first, i2.first) < 0.0f)
					return false;

				return true;
			}
		};

		template<typename Other>
		struct CollisionTest<OBB, Other>
		{
			static void update_collision(const OBB& c, const Other& other, CollisionResult& info)
			{
				std::array<UnitVector2D, 2> axes = c.get_axes();
				float depth = sat(c, other, axes[0]);
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
					return;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = axes[0].get_quarter_turn();
				}
				depth = sat(c, other, axes[1]);
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = axes[1].get_quarter_turn();
				}
			}
		};
	}
}
