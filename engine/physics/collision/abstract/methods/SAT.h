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
		struct CollisionTest
		{
			static_assert(false, "CollisionTest not supported for the provided shape combo.");
		};
	}

	template<typename Shape1, typename Shape2>
	inline OverlapResult overlaps(const Shape1& c1, const Shape2& c2)
	{
		return internal::CollisionTest<Shape1, Shape2>::overlaps_impl(c1, c2) && internal::CollisionTest<Shape2, Shape1>::overlaps_impl(c2, c1);
	}

	template<typename Shape1, typename Shape2>
	inline CollisionResult collides(const Shape1& c1, const Shape2& c2)
	{
		CollisionResult info{ .overlap = true, .penetration_depth = FLT_MAX };
		if (!internal::CollisionTest<Shape1, Shape2>::update_collision_result_impl(c1, c2, info))
			return info;
		internal::CollisionTest<Shape2, Shape1>::update_collision_result_impl(c2, c1, info);
		return info;
	}

	namespace internal
	{
		template<typename Shape1, typename Shape2>
		static float sat(const Shape1& c1, const Shape2& c2, glm::vec2 axis)
		{
			std::pair<float, float> i1 = c1.projection_interval(axis);
			std::pair<float, float> i2 = c2.projection_interval(axis);
			return std::min(i1.second, i2.second) - std::max(i1.first, i2.first);
		}

		template<typename Other>
		struct CollisionTest<ConvexHull, Other>
		{
			static OverlapResult overlaps_impl(const ConvexHull& c, const Other& other)
			{
				for (size_t i = 0; i < c.points.size(); ++i)
				{
					glm::vec2 p1 = c.points[i];
					glm::vec2 p2 = c.points[(i + 1) % c.points.size()];
					glm::vec2 edge = p2 - p1;
					glm::vec2 axis = glm::normalize(glm::vec2{ -edge.y, edge.x });
					if (sat(c, other, axis) < 0.0f)
						return false;
				}
				return true;
			}

			static bool update_collision_result_impl(const ConvexHull& c, const Other& other, CollisionResult& info)
			{
				for (size_t i = 0; i < c.points.size(); ++i)
				{
					glm::vec2 p1 = c.points[i];
					glm::vec2 p2 = c.points[(i + 1) % c.points.size()];
					glm::vec2 edge = p2 - p1;
					glm::vec2 axis = glm::normalize(glm::vec2{ -edge.y, edge.x });
					float depth = sat(c, other, axis);
					if (depth < 0.0f)
					{
						info.overlap = false;
						info.penetration_depth = 0.0f;
						return false;
					}
					else if (depth < info.penetration_depth)
					{
						info.penetration_depth = depth;
						info.unit_impulse = axis;
					}
				}
				return true;
			}
		};

		template<typename Other>
		static bool circle_update_collision_result_impl(const Circle& c, const Other& other, CollisionResult& info, glm::vec2 closest_point)
		{
			if (approx(closest_point, c.center))
				return true;
			glm::vec2 axis = glm::normalize(closest_point - c.center);
			float depth = sat(c, other, axis);
			if (depth < 0.0f)
			{
				info.overlap = false;
				info.penetration_depth = 0.0f;
				return false;
			}
			else if (depth < info.penetration_depth)
			{
				info.penetration_depth = depth;
				info.unit_impulse = axis;
			}
			return true;
		}

		template<typename Other>
		struct CollisionTest<Circle, Other>
		{
			static OverlapResult overlaps_impl(const Circle& c, const Other& other)
			{
				glm::vec2 closest_point = c.closest_point(other.points());
				return approx(closest_point, c.center) || sat(c, other, glm::normalize(closest_point - c.center)) >= 0.0f;
			}

			static bool update_collision_result_impl(const Circle& c, const Other& other, CollisionResult& info)
			{
				return circle_update_collision_result_impl(c, other, info, c.closest_point(other.points()));
			}
		};

		template<>
		struct CollisionTest<Circle, ConvexHull>
		{
			static OverlapResult overlaps_impl(const Circle& c, const ConvexHull& other)
			{
				glm::vec2 closest_point = c.closest_point(other.points);
				return approx(closest_point, c.center) || sat(c, other, glm::normalize(closest_point - c.center)) >= 0.0f;
			}

			static bool update_collision_result_impl(const Circle& c, const ConvexHull& other, CollisionResult& info)
			{
				return circle_update_collision_result_impl(c, other, info, c.closest_point(other.points));
			}
		};

		template<typename Other>
		struct CollisionTest<AABB, Other>
		{
			static OverlapResult overlaps_impl(const AABB& c, const Other& other)
			{
				return sat(c, other, { 1.0f, 0.0f }) >= 0.0f && sat(c, other, { 0.0f, 1.0f }) >= 0.0f;
			}

			static bool update_collision_result_impl(const AABB& c, const Other& other, CollisionResult& info)
			{
				float depth = sat(c, other, { 1.0f, 0.0f });
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
					return false;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = { 0.0f, 1.0f };
				}
				depth = sat(c, other, { 0.0f, 1.0f });
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
					return false;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = { -1.0f, 0.0f }; // TODO should this be { 1, 0 } in some cases? Test SAT extensively.
				}
				return true;
			}
		};

		template<typename Other>
		struct CollisionTest<OBB, Other>
		{
			static OverlapResult overlaps_impl(const OBB& c, const Other& other)
			{
				std::array<glm::vec2, 2> axes = c.get_axes();
				float cw = glm::dot(c.center, axes[0]);
				std::pair<float, float> i1 = { cw - 0.5f * c.width, cw + 0.5f * c.width };
				std::pair<float, float> i2 = projection_interval(other, axes[0]);
				if (axis_signed_overlap(i1.first, i1.second, i2.first, i2.second) < 0.0f)
					return false;
				float ch = glm::dot(c.center, axes[1]);
				i1 = { ch - 0.5f * c.height, ch + 0.5f * c.height };
				i2 = projection_interval(other, axes[1]);
				if (axis_signed_overlap(i1.first, i1.second, i2.first, i2.second) < 0.0f)
					return false;
				return true;
			}

			static bool update_collision_result_impl(const OBB& c, const Other& other, CollisionResult& info)
			{
				std::array<glm::vec2, 2> axes = c.get_axes();
				float depth = sat(c, other, axes[0]);
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
					return false;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = { -axes[1].y, axes[1].x };
				}
				depth = sat(c, other, axes[1]);
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
					return false;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = { -axes[1].y, axes[1].x };
				}
				return true;
			}
		};
	}
}
