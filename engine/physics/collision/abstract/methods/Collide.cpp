#include "Collide.h"

namespace oly::acm2d
{
	OverlapInfo overlap(const Circle& c1, const Circle& c2)
	{
		float dist_sqrd = math::mag_sqrd(c2.center - c1.center);
		float rsum = c1.radius + c2.radius;
		return dist_sqrd <= rsum * rsum;
	}

	GeometricInfo geometric_collision(const Circle& c1, const Circle& c2)
	{
		GeometricInfo info{};
		info.overlap = overlap(c1, c2);
		if (info.overlap)
		{
			info.penetration_depth = c1.radius + c2.radius - math::magnitude(c2.center - c1.center);
			info.unit_impulse = (c1.center != c2.center) ? glm::normalize(c2.center - c1.center) : glm::vec2{};
		}
		return info;
	}

	StructuralInfo structural_collision(const Circle& c1, const Circle& c2)
	{
		StructuralInfo info{};
		info.simple = geometric_collision(c1, c2);

		if (info.simple.overlap)
		{
			std::vector<StructuralInfo::ContactElement> contact_elements;

			if (info.simple.penetration_depth == 0)
			{
				float dist_sqrd = math::mag_sqrd(c2.center - c1.center);
				float theta = glm::acos((c1.radius * c1.radius + dist_sqrd - c2.radius * c2.radius) / (2 * c1.radius * glm::sqrt(dist_sqrd)));
				float alpha = glm::atan(c2.center.y - c1.center.y, c2.center.x - c1.center.x);
				contact_elements.resize(2);
				contact_elements[0] = c1.center + c1.radius * math::dir_vector(alpha + theta);
				contact_elements[1] = c1.center + c1.radius * math::dir_vector(alpha - theta);
			}
			else
				contact_elements = { { c1.center + c1.radius * glm::normalize(c2.center - c1.center) } };

			info.structure = contact_elements;
		}
		else
		{
			glm::vec2 d = glm::normalize(c2.center - c1.center);
			info.structure = std::make_pair(c1.center + c1.radius * d, c2.center - c2.radius * d);
		}

		return info;
	}

	OverlapInfo overlap(const AABB& c1, const AABB& c2)
	{
		return c1.x1 <= c2.x2 && c2.x1 <= c1.x2 && c1.y1 <= c2.y2 && c2.y1 <= c1.y2;
	}

	GeometricInfo geometric_collision(const AABB& c1, const AABB& c2)
	{
		GeometricInfo info{};
		info.overlap = overlap(c1, c2);

		if (info.overlap)
		{
			glm::vec2 depth;
			depth.x = std::min(c2.x2 - c1.x1, c1.x2 - c2.x1);
			depth.y = std::min(c2.y2 - c1.y1, c1.y2 - c2.y1);
			info.penetration_depth = glm::sqrt(depth.x * depth.x + depth.y * depth.y);
			if (depth.x == (c2.x2 - c1.x1))
				depth.x *= -1.0f;
			if (depth.y == (c2.y2 - c1.y1))
				depth.y *= -1.0f;
			info.unit_impulse = glm::normalize(depth);
		}

		return info;
	}

	StructuralInfo structural_collision(const AABB& c1, const AABB& c2)
	{
		StructuralInfo info{};
		info.simple = geometric_collision(c1, c2);

		if (info.simple.overlap)
		{
			std::vector<StructuralInfo::ContactElement> contact_elements;
			// TODO
			info.structure = contact_elements;
		}
		else
		{
			std::pair<glm::vec2, glm::vec2> witness_points;

			if (c1.x1 > c2.x2)
			{
				witness_points.first.x = c1.x1;
				witness_points.second.x = c2.x2;
			}
			else if (c2.x1 > c1.x2)
			{
				witness_points.first.x = c1.x2;
				witness_points.second.x = c2.x1;
			}
			else
			{
				float x = 0.0f;
				if (c1.x1 < c2.x1)
					x += c2.x1;
				else
					x += c1.x1;
				if (c1.x2 > c2.x2)
					x += c2.x2;
				else
					x += c1.x2;
				witness_points.second.x = witness_points.first.x = 0.5f * x;
			}

			if (c1.y1 > c2.y2)
			{
				witness_points.first.y = c1.y1;
				witness_points.second.y = c2.y2;
			}
			else if (c2.y1 > c1.y2)
			{
				witness_points.first.y = c1.y2;
				witness_points.second.y = c2.y1;
			}
			else
			{
				float y = 0.0f;
				if (c1.y1 < c2.y1)
					y += c2.y1;
				else
					y += c1.y1;
				if (c1.y2 > c2.y2)
					y += c2.y2;
				else
					y += c1.y2;
				witness_points.second.y = witness_points.first.y = 0.5f * y;
			}

			info.structure = witness_points;
		}

		return info;
	}
}
