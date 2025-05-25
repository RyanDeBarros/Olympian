#include "Circle.h"

namespace oly::acm2d
{
	namespace internal
	{
		const glm::mat3x2& CircleGlobalAccess::global(const Circle& c)
		{
			return c.global;
		}

		glm::mat3x2& CircleGlobalAccess::global(Circle& c)
		{
			return c.global;
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
		float center_proj = axis.dot(center);
		return { center_proj - radius, center_proj + radius };
	}

	glm::vec2 Circle::deepest_point(const UnitVector2D& axis) const
	{
		return center + radius * (glm::vec2)axis;
	}
}
