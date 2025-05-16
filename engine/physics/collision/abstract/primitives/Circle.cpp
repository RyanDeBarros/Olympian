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
}
