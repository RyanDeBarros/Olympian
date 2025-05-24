#include "Circle.h"

namespace oly::acm2d
{
	Circle Circle::fast_wrap(const math::Polygon2D& polygon)
	{
		Circle c{};

		c.center = {};
		for (glm::vec2 point : polygon.points)
			c.center += point;
		c.center /= (float)polygon.points.size();

		c.radius = 0.0f;
		for (glm::vec2 point : polygon.points)
			c.radius = std::max(c.radius, math::mag_sqrd(point - c.center));
		c.radius = glm::sqrt(c.radius);

		return c;
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
