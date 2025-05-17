#include "Collide.h"

namespace oly::acm2d
{
	bool point_trace(const Circle& c, glm::vec2 test)
	{
		return math::mag_sqrd(c.center - test) <= c.radius * c.radius;
	}

	static bool ray_contact_circle(const Circle& c, Ray ray, std::pair<float, float>& t_pair)
	{
		float cross = math::cross(ray.direction(), c.center - ray.origin);
		float discriminant = c.radius * c.radius - cross * cross;
		if (discriminant < 0.0f)
			return false;

		float offset = glm::dot(ray.direction(), c.center - ray.origin);
		discriminant = glm::sqrt(discriminant);
		t_pair.first = offset - discriminant;
		t_pair.second = offset + discriminant;

		// no forward contact
		if (t_pair.second < 0.0f)
			return false;

		// semi-infinite ray
		if (ray.clip == 0.0f)
			return true;

		// contact within clip
		if (t_pair.first <= ray.clip)
			return true;

		return false;
	}

	OverlapInfo ray_trace(const Circle& c, Ray ray)
	{
		std::pair<float, float> t_pair;
		return ray_contact_circle(c, ray, t_pair);
	}

	SimpleRayHit simple_ray_trace(const Circle& c, Ray ray)
	{
		std::pair<float, float> t_pair;
		if (!ray_contact_circle(c, ray, t_pair))
			return { .hit = false };

		SimpleRayHit info{ .hit = true };
		info.contact = std::max(t_pair.first, 0.0f) * ray.direction() + ray.origin;
		return info;
	}

	DeepRayHit deep_ray_trace(const Circle& c, Ray ray)
	{
		std::pair<float, float> t_pair;
		if (!ray_contact_circle(c, ray, t_pair))
			return { .hit = false };

		DeepRayHit info{ .hit = true };
		info.contact = std::max(t_pair.first, 0.0f) * ray.direction() + ray.origin;
		info.normal = glm::normalize(c.center - info.contact);
		info.exit = ray.clip == 0.0f || ray.clip > t_pair.second;

		if (ray.clip == 0.0f)
			info.depth = t_pair.second - t_pair.first;
		else
			info.depth = std::min(t_pair.second, ray.clip) - t_pair.first;

		return info;
	}

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
			info.penetration_depth = c1.radius + c2.radius - math::magnitude(c1.center - c2.center);
			info.unit_impulse = (c1.center != c2.center) ? glm::normalize(c1.center - c2.center) : glm::vec2{};
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

			info.static_manifold = info.active_manifold = contact_elements;
		}
		else
		{
			glm::vec2 d = glm::normalize(c2.center - c1.center);
			info.active_manifold = c1.center + c1.radius * d;
			info.static_manifold = c2.center - c2.radius * d;
		}

		return info;
	}

	bool point_trace(const AABB& c, glm::vec2 test)
	{
		return test.x >= c.x1 && test.x <= c.x2 && test.y >= c.y1 && test.y <= c.y2;
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
			depth.x = std::min(c1.x1 - c2.x2, c2.x1 - c1.x2);
			depth.y = std::min(c1.y1 - c2.y2, c2.y1 - c1.y2);
			info.penetration_depth = glm::sqrt(depth.x * depth.x + depth.y * depth.y);
			if (depth.x == (c1.x1 - c2.x2))
				depth.x *= -1.0f;
			if (depth.y == (c1.y1 - c2.y2))
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
			static const auto find_contact_elements = [](const AABB& c1, const AABB& c2) {
				std::vector<StructuralInfo::ContactElement> contact_elements;

				// bottom edge
				if (c1.y1 >= c2.y1)
				{
					float left = std::max(c1.x1, c2.x1);
					float right = std::min(c1.x2, c2.x2);
					if (left == right)
						contact_elements.push_back(glm::vec2{ left, c1.y1 });
					else
						contact_elements.push_back(StructuralInfo::Line{ { left, c1.y1 }, { right, c1.y1 } });
				}

				// top edge
				if (c1.y2 <= c2.y2)
				{
					float left = std::max(c1.x1, c2.x1);
					float right = std::min(c1.x2, c2.x2);
					if (left == right)
						contact_elements.push_back(glm::vec2{ left, c1.y2 });
					else
						contact_elements.push_back(StructuralInfo::Line{ { left, c1.y2 }, { right, c1.y2 } });
				}

				// left edge
				if (c1.x1 >= c2.x1)
				{
					float bottom = std::max(c1.y1, c2.y1);
					float top = std::min(c1.y2, c2.y2);
					if (bottom == top)
						contact_elements.push_back(glm::vec2{ c1.x1, bottom });
					else
						contact_elements.push_back(StructuralInfo::Line{ { c1.x1, bottom }, { c1.x1, top } });
				}

				// right edge
				if (c1.x2 <= c2.x2)
				{
					float bottom = std::max(c1.y1, c2.y1);
					float top = std::min(c1.y2, c2.y2);
					if (bottom == top)
						contact_elements.push_back(glm::vec2{ c1.x2, bottom });
					else
						contact_elements.push_back(StructuralInfo::Line{ { c1.x2, bottom }, { c1.x2, top } });
				}

				return contact_elements;
				};

			info.active_manifold = find_contact_elements(c1, c2);
			info.static_manifold = find_contact_elements(c2, c1);
		}
		else
		{
			glm::vec2 active_witness{};
			glm::vec2 static_witness{};

			if (c1.x1 > c2.x2)
			{
				active_witness.x = c1.x1;
				static_witness.x = c2.x2;
			}
			else if (c2.x1 > c1.x2)
			{
				active_witness.x = c1.x2;
				static_witness.x = c2.x1;
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
				static_witness.x = active_witness.x = 0.5f * x;
			}

			if (c1.y1 > c2.y2)
			{
				active_witness.y = c1.y1;
				static_witness.y = c2.y2;
			}
			else if (c2.y1 > c1.y2)
			{
				active_witness.y = c1.y2;
				static_witness.y = c2.y1;
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
				static_witness.y = active_witness.y = 0.5f * y;
			}

			info.active_manifold = active_witness;
			info.static_manifold = static_witness;
		}

		return info;
	}
}
