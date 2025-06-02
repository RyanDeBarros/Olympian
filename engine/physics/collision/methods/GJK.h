#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "core/math/Geometry.h"
#include "core/types/Approximate.h"
#include "core/base/Errors.h"
#include "core/util/Logger.h"

#include <optional>

namespace oly::col2d::gjk
{
	inline static constexpr size_t VERTICES_THRESHOLD = 16;

	template<typename Shape1, typename Shape2>
	inline glm::vec2 support(const Shape1& c1, const Shape2& c2, const UnitVector2D& axis)
	{
		return c1.deepest_point(axis) - c2.deepest_point(-axis);
	}

	inline bool handle_simplex(std::vector<glm::vec2>& simplex, UnitVector2D& axis)
	{
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
			if (glm::dot(ab_ortho, -a) > 0.0f)
			{
				// remove c
				simplex.erase(simplex.begin());
				axis = ab_ortho;
				return false;
			}

			glm::vec2 ac_ortho = math::triple_cross(b - a, c - a, c - a);
			if (glm::dot(ac_ortho, -a) > 0.0f)
			{
				// remove b
				simplex.erase(simplex.begin() + 1);
				axis = ac_ortho;
				return false;
			}

			// origin inside triangle
			return true;
		}
		return false;
	}

	template<typename Shape1, typename Shape2>
	inline OverlapResult overlaps(const Shape1& c1, const Shape2& c2, size_t max_iterations = 20)
	{
		UnitVector2D axis;
		std::vector<glm::vec2> simplex;
		simplex.push_back(support(c1, c2, axis));
		axis = -simplex.back();

		for (size_t i = 0; i < max_iterations; ++i)
		{
			glm::vec2 p = support(c1, c2, axis);
			if (axis.dot(p) < 0.0f)
				return false;

			simplex.push_back(p);
			if (handle_simplex(simplex, axis))
				return true;
		}

		LOG << LOG.begin_temp(LOG.warning) << LOG.start << "GJK overflow" << LOG.nl << LOG.end_temp;
		throw Error(ErrorCode::GJK_OVERFLOW);
	}

	template<typename Shape1, typename Shape2>
	inline CollisionResult collides(const Shape1& c1, const Shape2& c2, size_t gjk_max_iterations = 20, size_t epa_max_iterations = 64, float epa_epsilon = 1e-5f)
	{
		UnitVector2D axis;
		std::vector<glm::vec2> simplex;
		simplex.push_back(support(c1, c2, axis));
		axis = -simplex.back();
		bool intersecting = false;

		for (size_t i = 0; i < gjk_max_iterations; ++i)
		{
			glm::vec2 p = support(c1, c2, axis);
			if (axis.dot(p) < 0.0f)
				return { .overlap = false };

			simplex.push_back(p);
			if (handle_simplex(simplex, axis))
			{
				intersecting = true;
				break;
			}
		}
		if (!intersecting)
		{
			LOG << LOG.begin_temp(LOG.warning) << LOG.start << "GJK overflow" << LOG.nl << LOG.end_temp;
			throw Error(ErrorCode::GJK_OVERFLOW);
		}

		// EPA

		struct Edge
		{
			size_t index;
			float distance;
			UnitVector2D normal;
		};

		static const auto find_closest_edge = [](const std::vector<glm::vec2>& polygon) -> Edge {
			float min_dist = std::numeric_limits<float>::max();
			Edge closest{};
			for (size_t i = 0; i < polygon.size(); ++i)
			{
				glm::vec2 a = polygon[i];
				glm::vec2 b = polygon[(i + 1) % polygon.size()];
				UnitVector2D normal = UnitVector2D(b - a).get_quarter_turn();
				
				float dist = normal.dot(a);
				if (dist < min_dist)
				{
					min_dist = dist;
					closest = { i, dist, normal };
				}
			}
			return closest;
			};

		for (size_t i = 0; i < epa_max_iterations; ++i)
		{
			Edge edge = find_closest_edge(simplex);
			glm::vec2 p = support(c1, c2, edge.normal);
			float d = edge.normal.dot(p);

			if (near_zero(d - edge.distance, epa_epsilon))
				return { .overlap = true, .penetration_depth = d, .unit_impulse = -edge.normal };

			simplex.insert(simplex.begin() + edge.index + 1, p);
		}

		LOG << LOG.begin_temp(LOG.warning) << LOG.start << "EPA overflow" << LOG.nl << LOG.end_temp;
		throw Error(ErrorCode::EPA_OVERFLOW);
	}

	template<typename Shape1, typename Shape2>
	inline ContactResult contacts(const Shape1& c1, const Shape2& c2, size_t gjk_max_iterations = 20, size_t epa_max_iterations = 64, float epa_epsilon = 1e-5f)
	{
		CollisionResult collision = collides(c1, c2, gjk_max_iterations, epa_max_iterations, epa_epsilon);
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
}
