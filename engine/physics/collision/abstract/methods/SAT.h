#pragma once

#include "physics/collision/abstract/methods/CollisionInfo.h"
#include "physics/collision/abstract/primitives/ConvexHull.h"
#include "physics/collision/abstract/primitives/Circle.h"
#include "physics/collision/abstract/primitives/AABB.h"
#include "physics/collision/abstract/primitives/OBB.h"
#include "physics/collision/abstract/primitives/KDOP.h"

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
				CollisionResult info{ .overlap = true, .penetration_depth = std::numeric_limits<float>::max() };
				internal::CollisionTest<Shape1, Shape2>::update_collision(c1, c2, info);
				if (!info.overlap)
					return info;
				internal::CollisionTest<Shape2, Shape1>::update_collision(c2, c1, info);
				if (!info.overlap)
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
		template<typename Shape1, typename Shape2>
		static float sat(const Shape1& c1, const Shape2& c2, UnitVector2D& axis)
		{
			std::pair<float, float> i1 = c1.projection_interval(axis);
			std::pair<float, float> i2 = c2.projection_interval(axis);
			if (i1.first > i2.first)
				axis = -axis;
			return std::min(i1.second, i2.second) - std::max(i1.first, i2.first);
		}

		template<typename Other>
		struct OverlapTest<ConvexHull, Other>
		{
			static OverlapResult impl(const ConvexHull& c, const Other& other)
			{
				for (size_t i = 0; i < c.points.size(); ++i)
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
				UnitVector2D axis = c.get_axis_1();
				float cw = axis.dot(c.center);
				std::pair<float, float> i1 = { cw - 0.5f * c.width, cw + 0.5f * c.width };
				std::pair<float, float> i2 = other.projection_interval(axis);
				if (std::min(i1.second, i2.second) - std::max(i1.first, i2.first) < 0.0f)
					return false;

				axis = c.get_axis_2();
				float ch = axis.dot(c.center);
				i1 = { ch - 0.5f * c.height, ch + 0.5f * c.height };
				i2 = other.projection_interval(axis);
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
				UnitVector2D axis = c.get_axis_1();
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
					info.unit_impulse = axis.get_quarter_turn();
				}
				axis = c.get_axis_2();
				depth = sat(c, other, axis);
				if (depth < 0.0f)
				{
					info.overlap = false;
					info.penetration_depth = 0.0f;
				}
				else if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					info.unit_impulse = axis.get_quarter_turn();
				}
			}
		};

		template<size_t K_half, typename Other>
		struct OverlapTest<KDOP<K_half>, Other>
		{
			static OverlapResult impl(const KDOP<K_half>& c, const Other& other)
			{
				// only go through half the axes, since the other half has parallel normals
				for (size_t i = 0; i < K_half; ++i)
				{
					std::pair<float, float> i2 = other.projection_interval(KDOP<K_half>::uniform_axis(i));
					if (std::min(c.get_maximum(i), i2.second) - std::max(c.get_minimum(i), i2.first) < 0.0f)
						return false;
				}
				return true;
			}
		};

		template<size_t K_half, typename Other>
		struct CollisionTest<KDOP<K_half>, Other>
		{
			static void update_collision(const KDOP<K_half>& c, const Other& other, CollisionResult& info)
			{
				// only go through half the axes, since the other half has parallel normals
				for (size_t i = 0; i < K_half; ++i)
				{
					std::pair<float, float> i1 = { c.get_minimum(i), c.get_maximum(i) };
					std::pair<float, float> i2 = other.projection_interval(KDOP<K_half>::uniform_axis(i));
					float depth = std::min(i1.second, i2.second) - std::max(i1.first, i2.first);
					if (depth < 0.0f)
					{
						info.overlap = false;
						info.penetration_depth = 0.0f;
						return;
					}
					else if (depth < info.penetration_depth)
					{
						info.penetration_depth = depth;
						info.unit_impulse = i1.first < i2.first ? KDOP<K_half>::uniform_axis(i) : -KDOP<K_half>::uniform_axis(i);
					}
				}
			}
		};

		template<size_t K_half>
		struct FullOverlapTest<KDOP<K_half>, KDOP<K_half>>
		{
			static OverlapResult call(const KDOP<K_half>& c1, const KDOP<K_half>& c2)
			{
				// only go through half the axes, since the other half has parallel normals
				for (size_t i = 0; i < K_half; ++i)
				{
					if (std::min(c1.get_maximum(i), c2.get_maximum(i)) - std::max(c1.get_minimum(i), c2.get_minimum(i)) < 0.0f)
						return false;
				}
				return true;
			}
		};

		template<size_t K_half>
		struct FullCollisionTest<KDOP<K_half>, KDOP<K_half>>
		{
			static CollisionResult call(const KDOP<K_half>& c1, const KDOP<K_half>& c2)
			{
				CollisionResult info{ .overlap = true, .penetration_depth = std::numeric_limits<float>::max() };
				// only go through half the axes, since the other half has parallel normals
				for (size_t i = 0; i < K_half; ++i)
				{
					std::pair<float, float> i1 = { c1.get_minimum(i), c1.get_maximum(i) };
					std::pair<float, float> i2 = { c2.get_minimum(i), c2.get_maximum(i) };
					float depth = std::min(i1.second, i2.second) - std::max(i1.first, i2.first);
					if (depth < 0.0f)
						return { .overlap = false };
					else if (depth < info.penetration_depth)
					{
						info.penetration_depth = depth;
						info.unit_impulse = i1.first < i2.first ? KDOP<K_half>::uniform_axis(i) : -KDOP<K_half>::uniform_axis(i);
					}
				}
				return info;
			}
		};

		template<typename Other>
		struct OverlapTest<CustomKDOP, Other>
		{
			static OverlapResult impl(const CustomKDOP& c, const Other& other)
			{
				// only go through half the axes, since the other half has parallel normals
				for (size_t i = 0; i < c.get_k_half(); ++i)
				{
					std::pair<float, float> i2 = other.projection_interval(c.edge_normal(i));
					if (std::min(c.get_maximum(i), i2.second) - std::max(c.get_minimum(i), i2.first) < 0.0f)
						return false;
				}
				return true;
			}
		};

		template<typename Other>
		struct CollisionTest<CustomKDOP, Other>
		{
			static void update_collision(const CustomKDOP& c, const Other& other, CollisionResult& info)
			{
				// only go through half the axes, since the other half has parallel normals
				for (size_t i = 0; i < c.get_k_half(); ++i)
				{
					std::pair<float, float> i1 = { c.get_minimum(i), c.get_maximum(i) };
					std::pair<float, float> i2 = other.projection_interval(c.edge_normal(i));
					float depth = std::min(i1.second, i2.second) - std::max(i1.first, i2.first);
					if (depth < 0.0f)
					{
						info.overlap = false;
						info.penetration_depth = 0.0f;
						return;
					}
					else if (depth < info.penetration_depth)
					{
						info.penetration_depth = depth;
						info.unit_impulse = i1.first < i2.first ? c.edge_normal(i) : -c.edge_normal(i);
					}
				}
			}
		};

		template<>
		struct FullOverlapTest<CustomKDOP, CustomKDOP>
		{
			static OverlapResult call(const CustomKDOP& c1, const CustomKDOP& c2)
			{
				if (c1.get_k_half() == c2.get_k_half() && c1.get_axes() == c2.get_axes())
				{
					// only go through half the axes, since the other half has parallel normals
					for (size_t i = 0; i < c1.get_k_half(); ++i)
					{
						if (std::min(c1.get_maximum(i), c2.get_maximum(i)) - std::max(c1.get_minimum(i), c2.get_minimum(i)) < 0.0f)
							return false;
					}
					return true;
				}
				else
				{
					return internal::OverlapTest<CustomKDOP, CustomKDOP>::impl(c1, c2) && internal::OverlapTest<CustomKDOP, CustomKDOP>::impl(c2, c1);
				}
			}
		};

		template<>
		struct FullCollisionTest<CustomKDOP, CustomKDOP>
		{
			static CollisionResult call(const CustomKDOP& c1, const CustomKDOP& c2)
			{
				if (c1.get_k_half() == c2.get_k_half() && c1.get_axes() == c2.get_axes())
				{
					CollisionResult info{ .overlap = true, .penetration_depth = std::numeric_limits<float>::max() };
					// only go through half the axes, since the other half has parallel normals
					for (size_t i = 0; i < c1.get_k_half(); ++i)
					{
						std::pair<float, float> i1 = { c1.get_minimum(i), c1.get_maximum(i) };
						std::pair<float, float> i2 = { c2.get_minimum(i), c2.get_maximum(i) };
						float depth = std::min(i1.second, i2.second) - std::max(i1.first, i2.first);
						if (depth < 0.0f)
							return { .overlap = false };
						else if (depth < info.penetration_depth)
						{
							info.penetration_depth = depth;
							info.unit_impulse = i1.first < i2.first ? c1.edge_normal(i) : -c1.edge_normal(i);
						}
					}
					return info;
				}
				else
				{
					CollisionResult info{ .overlap = true, .penetration_depth = std::numeric_limits<float>::max() };
					internal::CollisionTest<CustomKDOP, CustomKDOP>::update_collision(c1, c2, info);
					if (!info.overlap)
						return info;
					internal::CollisionTest<CustomKDOP, CustomKDOP>::update_collision(c2, c1, info);
					if (!info.overlap)
						return info;
					return info;
				}
			}
		};
	}
}
