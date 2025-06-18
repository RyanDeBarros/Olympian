#include "Circle.h"

#include "core/base/Transforms.h"
#include "core/base/Errors.h"

#include "physics/collision/elements/OBB.h"

namespace oly::col2d
{
	namespace internal
	{
		static void invert(glm::mat3x2& g, glm::mat3x2& ginv)
		{
			float det = g[0][0] * g[1][1] - g[0][1] * g[1][0];
			if (det == 0.0f)
			{
				g = 0.0f;
				ginv = 0.0f;
				throw Error(ErrorCode::BAD_COLLISION_SHAPE);
			}

			det = 1.0f / det;
			ginv = { { g[1][1] * det, -g[0][1] * det }, { -g[1][0] * det, g[0][0] * det }, { (g[1][0] * g[2][1] - g[1][1] * g[2][0]) * det, (g[0][1] * g[2][0] - g[0][0] * g[2][1]) * det} };
		}

		const glm::mat3x2& CircleGlobalAccess::get_global(const Circle& c)
		{
			return c.global;
		}

		const glm::mat3x2& CircleGlobalAccess::get_ginv(const Circle& c)
		{
			return c.ginv;
		}

		void CircleGlobalAccess::set_global(Circle& c, const glm::mat3x2& g)
		{
			c.global = g;
			invert(c.global, c.ginv);
		}

		void CircleGlobalAccess::set_ginv(Circle& c, const glm::mat3x2& ginv)
		{
			c.ginv = ginv;
			invert(c.ginv, c.global);
		}

		bool CircleGlobalAccess::has_no_global(const Circle& c)
		{
			return c.global == DEFAULT;
		}

		float CircleGlobalAccess::radius_disparity(const Circle& c)
		{
			// DOC
			glm::vec2 l0 = c.global[0];
			glm::vec2 l1 = c.global[1];
			float mag_sqrd_diff = glm::dot(l0, l0) - glm::dot(l1, l1);
			return glm::sqrt(mag_sqrd_diff * mag_sqrd_diff + 4 * glm::dot(l0, l1) * glm::dot(l0, l1));
		}

		float CircleGlobalAccess::max_radius(const Circle& c)
		{
			// DOC
			return 0.5f * c.radius * (glm::dot(c.global[0], c.global[0]) + glm::dot(c.global[1], c.global[1]) + radius_disparity(c));
		}

		float CircleGlobalAccess::min_radius(const Circle& c)
		{
			// DOC
			return 0.5f * c.radius * (glm::dot(c.global[0], c.global[0]) + glm::dot(c.global[1], c.global[1]) - radius_disparity(c));
		}

		Circle CircleGlobalAccess::bounding_circle(const Circle& c)
		{
			// DOC
			return Circle(global_center(c), max_radius(c));
		}

		OBB CircleGlobalAccess::bounding_obb(const Circle& c)
		{
			// DOC
			float max_angle = 0.5f * glm::atan(2.0f * glm::dot(c.global[0], c.global[1]), glm::dot(c.global[0], c.global[0]) - glm::dot(c.global[1], c.global[1]));
			return OBB{ .center = global_center(c), .width = 2.0f * max_radius(c), .height = 2.0f * min_radius(c), .rotation = max_angle };
		}

		glm::vec2 CircleGlobalAccess::global_center(const Circle& c)
		{
			return transform_point(c.global, c.center);
		}
	}

	Circle Circle::fast_wrap(const math::Polygon2D& polygon)
	{
		glm::vec2 center = {};
		for (glm::vec2 point : polygon)
			center += point;
		center /= (float)polygon.size();

		float radius = 0.0f;
		for (glm::vec2 point : polygon)
			radius = std::max(radius, math::mag_sqrd(point - center));
		radius = glm::sqrt(radius);

		return Circle(center, radius);
	}

	std::pair<float, float> Circle::projection_interval(const UnitVector2D& axis) const
	{
		if (global == internal::CircleGlobalAccess::DEFAULT)
		{
			float center_proj = axis.dot(center);
			return { center_proj - radius, center_proj + radius };
		}
		else
		{
			glm::vec2 global_center = transform_point(global, center);
			float offset = axis.dot(global_center);
			float multiplier = glm::length(glm::transpose(glm::mat2(global)) * axis);
			return { offset - radius * multiplier, offset + radius * multiplier };
		}
	}

	float Circle::projection_min(const UnitVector2D& axis) const
	{
		if (global == internal::CircleGlobalAccess::DEFAULT)
			return axis.dot(center) - radius;
		else
			return axis.dot(transform_point(global, center)) - radius * glm::length(glm::transpose(glm::mat2(global)) * axis);
	}

	float Circle::projection_max(const UnitVector2D& axis) const
	{
		if (global == internal::CircleGlobalAccess::DEFAULT)
			return axis.dot(center) + radius;
		else
			return axis.dot(transform_point(global, center)) + radius * glm::length(glm::transpose(glm::mat2(global)) * axis);
	}

	glm::vec2 Circle::deepest_point(const UnitVector2D& axis) const
	{
		if (global == internal::CircleGlobalAccess::DEFAULT)
			return center + radius * (glm::vec2)axis;
		else
			return radius * (glm::vec2)axis * math::inv_magnitude(glm::mat2(ginv) * axis) + transform_point(global, center);
	}
}
