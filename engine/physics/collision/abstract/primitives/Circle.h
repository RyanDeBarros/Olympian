#pragma once

#include "core/math/Geometry.h"

namespace oly::acm2d
{
	struct Circle
	{
		glm::vec2 center;
		float radius;

		float area() const { return glm::pi<float>() * radius * radius; }

		static Circle fast_wrap(const math::Polygon2D& polygon);

		std::pair<float, float> projection_interval(glm::vec2 axis) const;

		template<typename Polygon>
		glm::vec2 closest_point(const Polygon& polygon) const
		{
			float closest_dist_sqrd = FLT_MAX;
			glm::vec2 cp{};
			for (glm::vec2 point : polygon)
			{
				float dist_sqrd = math::mag_sqrd(center - point);
				if (dist_sqrd < closest_dist_sqrd)
				{
					closest_dist_sqrd = dist_sqrd;
					cp = point;
				}
			}
			return cp;
		}
	};
}
