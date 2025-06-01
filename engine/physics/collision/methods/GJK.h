#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "core/math/Geometry.h"
#include "core/types/Approximate.h"
#include "core/base/Errors.h"

#include <optional>

namespace oly::col2d::gjk
{
	template<typename Shape1, typename Shape2>
	inline OverlapResult overlaps(const Shape1& c1, const Shape2& c2, size_t max_iterations = 20)
	{
		static const auto support = [](const Shape1& c1, const Shape2& c2, const UnitVector2D& axis) -> glm::vec2 {
			return c1.deepest_point(axis) + c2.deepest_point(-axis);
			};

		static const auto handle_simplex = [](std::vector<glm::vec2>& simplex, UnitVector2D& axis) -> bool {
			if (simplex.size() == 2)
			{
				// line segment case
				glm::vec2 a = simplex[1];
				glm::vec2 b = simplex[0];
				
				// dir is orthogonal to (b - a) pointing towards origin
				glm::vec2 dir = math::triple_cross(b - a, -a, b - a);
				if (near_zero(dir))
					axis = UnitVector2D(b - a).get_quarter_turn();
				else
					axis = dir;
			}
			else if (simplex.size() == 3)
			{
				// triangle case
				glm::vec2 a = simplex[2];
				glm::vec2 b = simplex[1];
				glm::vec2 c = simplex[0];

				glm::vec2 ab_ortho = math::triple_cross(c - a, b - a, b - a);
				glm::vec2 ac_ortho = math::triple_cross(b - a, c - a, c - a);
				if (glm::dot(ab_ortho, -a) > 0.0f)
				{
					// remove c
					simplex.erase(simplex.begin());
					axis = ab_ortho;
				}
				else if (glm::dot(ac_ortho, -a) > 0.0f)
				{
					// remove b
					simplex.erase(simplex.begin() + 1);
					axis = ac_ortho;
				}
				else
				{
					// origin inside triangle
					return true;
				}
			}
			return false;
		};

		UnitVector2D axis;
		std::vector<glm::vec2> simplex;
		simplex.push_back(support(c1, c2, axis));
		axis = -simplex.back();

		for (size_t i = 0; i < max_iterations; ++i)
		{
			glm::vec2 p = support(c1, c2, axis);
			if (axis.dot(p) <= 0.0f)
				return false;

			simplex.push_back(p);
			if (handle_simplex(simplex, axis))
				return true;
		}
		throw Error(ErrorCode::GJK_OVERFLOW);
	}

	template<typename Shape1, typename Shape2>
	inline CollisionResult collides(const Shape1& c1, const Shape2& c2)
	{
		// TODO
		throw Error(ErrorCode::GJK_OVERFLOW);
	}

	template<typename Shape1, typename Shape2>
	inline ContactResult contacts(const Shape1& c1, const Shape2& c2)
	{
		// TODO
		throw Error(ErrorCode::GJK_OVERFLOW);
	}
}
