#include "Collide.h"

namespace oly::acm2d
{
	bool collide(const Circle& c1, const Circle& c2, CollisionInfo* info)
	{
		float dist_sqrd = math::mag_sqrd(c2.center - c1.center);
		float rsum = c1.radius + c2.radius;
		bool overlap = (dist_sqrd <= rsum * rsum);

		if (info)
		{
			info->colliding = overlap;
			float dist = glm::sqrt(dist_sqrd);
			info->depth = rsum - dist;
			info->normal = c2.center - c1.center;
			if (overlap)
			{
				if (info->depth == 0)
				{
					float theta = glm::acos((c1.radius * c1.radius + dist_sqrd - c2.radius * c2.radius) / (2 * c1.radius * dist));
					float alpha = glm::atan(c2.center.y - c1.center.y, c2.center.x - c1.center.x);
					info->contact_points.resize(2);
					info->contact_points[0] = c1.center + c1.radius * math::dir_vector(alpha + theta);
					info->contact_points[1] = c1.center + c1.radius * math::dir_vector(alpha - theta);
				}
				else
					info->contact_points = { c1.center + c1.radius * glm::normalize(c2.center - c1.center) };
			}
			else
			{
				glm::vec2 d = glm::normalize(c2.center - c1.center);
				info->witness_points = { c1.center + c1.radius * d, c2.center - c2.radius * d };
			}
		}

		return overlap;
	}
}
