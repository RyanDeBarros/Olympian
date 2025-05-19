#include "Collide.h"

#include "core/types/Approximate.h"

namespace oly::acm2d
{
	OverlapResult point_hits(const Circle& c, glm::vec2 test)
	{
		return math::mag_sqrd(c.center - test) <= c.radius * c.radius;
	}

	static OverlapResult ray_contact_circle(const Circle& c, Ray ray, float& t1, float& t2)
	{
		float cross = math::cross(ray.direction(), c.center - ray.origin);
		float discriminant = c.radius * c.radius - cross * cross;
		if (discriminant < 0.0f)
			return false;

		float offset = glm::dot(ray.direction(), c.center - ray.origin);
		discriminant = glm::sqrt(discriminant);
		t1 = offset - discriminant;
		t2 = offset + discriminant;

		// no forward contact
		if (t2 < 0.0f)
			return false;

		// semi-infinite ray
		if (ray.clip == 0.0f)
			return true;

		// contact within clip
		if (t1 <= ray.clip)
			return true;

		return false;
	}

	static OverlapResult ray_contact_circle(const Circle& c, Ray ray)
	{
		float t1, t2;
		return ray_contact_circle(c, ray, t1, t2);
	}

	OverlapResult ray_hits(const Circle& c, Ray ray)
	{
		return ray_contact_circle(c, ray);
	}

	RaycastResult raycast(const Circle& c, Ray ray)
	{
		float t1, t2;
		if (!ray_contact_circle(c, ray, t1, t2))
			return { .hit = false };

		RaycastResult info{ .hit = true };
		info.contact = std::max(t1, 0.0f) * ray.direction() + ray.origin;
		return info;
	}

	OverlapResult overlaps(const Circle& c1, const Circle& c2)
	{
		float dist_sqrd = math::mag_sqrd(c2.center - c1.center);
		float rsum = c1.radius + c2.radius;
		return dist_sqrd <= rsum * rsum;
	}

	CollisionResult collides(const Circle& c1, const Circle& c2)
	{
		CollisionResult info{};
		info.overlap = overlaps(c1, c2);
		if (info.overlap)
		{
			info.penetration_depth = c1.radius + c2.radius - math::magnitude(c1.center - c2.center);
			info.unit_impulse = (c1.center != c2.center) ? glm::normalize(c1.center - c2.center) : glm::vec2{};
		}
		return info;
	}

	ContactResult contacts(const Circle& c1, const Circle& c2)
	{
		ContactResult info{};
		info.overlap = overlaps(c1, c2);

		glm::vec2 d = glm::normalize(c2.center - c1.center);
		info.active_feature.position = c1.center + c1.radius * d;
		info.static_feature.position = c2.center - c2.radius * d;

		if (info.overlap)
		{
			info.active_feature.impulse = glm::normalize(c1.center - c2.center) * (c1.radius + c2.radius - math::magnitude(c1.center - c2.center));
			info.static_feature.impulse = -info.active_feature.impulse;
		}

		return info;
	}

	OverlapResult point_hits(const AABB& c, glm::vec2 test)
	{
		return test.x >= c.x1 && test.x <= c.x2 && test.y >= c.y1 && test.y <= c.y2;
	}

	static OverlapResult ray_contact_line_segment(glm::vec2 a, glm::vec2 b, Ray ray, float& t1, float& t2)
	{
		if (math::cross(ray.direction(), a - b) != 0.0f) // ray and line segment are not parallel
		{
			// solution to linear system
			glm::vec2 q = glm::inverse(glm::mat2(ray.direction(), a - b)) * (a - ray.origin);

			// solution is not on line segment
			if (q.y < 0.0f || q.y > 1.0f)
				return false;

			// ray is pointing away from line segment
			if (q.x < 0.0f)
				return false;

			// single intersection point
			t2 = t1 = q.x;

			// semi-infinite ray
			if (ray.clip == 0.0f)
				return true;

			// ray reaches line segment
			return q.x <= ray.clip;
		}
		else if (near_zero(math::cross(b - a, ray.origin - a))) // ray's origin lies on infinite line
		{
			if (near_zero(ray.direction().y))
			{
				float ts[2]{
					(a.x - ray.origin.x) / ray.direction().x,
					(b.x - ray.origin.x) / ray.direction().x
				};
				t1 = std::min(ts[0], ts[1]);
				t2 = std::max(ts[0], ts[1]);
			}
			else
			{
				float ts[2]{
					(a.y - ray.origin.y) / ray.direction().y,
					(b.y - ray.origin.y) / ray.direction().y
				};
				t1 = std::min(ts[0], ts[1]);
				t2 = std::max(ts[0], ts[1]);
			}

			// ray is pointing away from line segment
			if (t2 < 0.0f)
				return false;

			t1 = std::max(t1, 0.0f);

			// semi-infinite ray
			if (ray.clip == 0.0f)
				return true;

			// ray reaches line segment
			return t1 <= ray.clip;
		}
		else
			return false;
	}

	static OverlapResult ray_contact_line_segment(glm::vec2 a, glm::vec2 b, Ray ray)
	{
		float t1, t2;
		return ray_contact_line_segment(a, b, ray, t1, t2);
	}

	OverlapResult ray_hits(const AABB& c, Ray ray)
	{
		// origin is inside AABB
		if (point_hits(c, ray.origin))
			return true;
		// check if ray hits any of AABB's edges
		return ray_contact_line_segment({ c.x1, c.y1 }, { c.x2, c.y1 }, ray) || ray_contact_line_segment({ c.x1, c.y2 }, { c.x2, c.y2 }, ray)
			|| ray_contact_line_segment({ c.x1, c.y1 }, { c.x1, c.y2 }, ray) || ray_contact_line_segment({ c.x2, c.y1 }, { c.x2, c.y2 }, ray);
	}

	RaycastResult raycast(const AABB& c, Ray ray)
	{
		if (point_hits(c, ray.origin))
			return { .hit = true, .contact = ray.origin };

		static const auto first_contact_on_horizontal_edge = [](glm::vec2 ray_origin, float t1, float t2, float x1, float x2, float y) -> RaycastResult {
			if (t1 == t2) // single intersection point
				return { .hit = true, .contact = t1 * glm::vec2{ x2 - x1, 0 } + glm::vec2{ x1, y } };
			else // ray is parallel to edge
			{
				if (ray_origin.x <= x1) // left edge corner
					return { .hit = true, .contact = { x1, y } };
				else if (ray_origin.x >= x2) // right edge corner
					return { .hit = true, .contact = { x2, y } };
				else // should be unreachable
					return { .hit = true, .contact = ray_origin };
			}
			};

		static const auto first_contact_on_vertical_edge = [](glm::vec2 ray_origin, float t1, float t2, float y1, float y2, float x) -> RaycastResult {
			if (t1 == t2) // single intersection point
				return { .hit = true, .contact = t1 * glm::vec2{ 0, y2 - y1 } + glm::vec2{ x, y1 } };
			else // ray is parallel to edge
			{
				if (ray_origin.y <= y1) // bottom edge corner
					return { .hit = true, .contact = { x, y1 } };
				else if (ray_origin.y >= y2) // top edge corner
					return { .hit = true, .contact = { x, y2 } };
				else // should be unreachable
					return { .hit = true, .contact = ray_origin };
			}
			};

		float t1, t2;

		// contact on bottom edge
		if (ray_contact_line_segment({ c.x1, c.y1 }, { c.x2, c.y1 }, ray, t1, t2))
		{
			if (ray.origin.y <= c.y1) // first contact on bottom edge
				return first_contact_on_horizontal_edge(ray.origin, t1, t2, c.x1, c.x2, c.y1);
			else if (ray_contact_line_segment({ c.x1, c.y2 }, { c.x2, c.y2 }, ray, t1, t2)) // first contact on top edge
				return first_contact_on_horizontal_edge(ray.origin, t1, t2, c.x1, c.x2, c.y2);
			else if (ray_contact_line_segment({ c.x1, c.y1 }, { c.x1, c.y2 }, ray, t1, t2)) // first contact on left edge
				return first_contact_on_vertical_edge(ray.origin, t1, t2, c.y1, c.y2, c.x1);
			else if (ray_contact_line_segment({ c.x2, c.y1 }, { c.x2, c.y2 }, ray, t1, t2)) // first contact on right edge
				return first_contact_on_vertical_edge(ray.origin, t1, t2, c.y1, c.y2, c.x2);
			else // should be unreachable
				return { .hit = false };
		}

		// contact on top edge
		if (ray_contact_line_segment({ c.x1, c.y2 }, { c.x2, c.y2 }, ray, t1, t2))
		{
			if (ray.origin.y >= c.y2) // first contact on top edge
				return first_contact_on_horizontal_edge(ray.origin, t1, t2, c.x1, c.x2, c.y2);
			else if (ray_contact_line_segment({ c.x1, c.y1 }, { c.x2, c.y1 }, ray, t1, t2)) // first contact on bottom edge
				return first_contact_on_horizontal_edge(ray.origin, t1, t2, c.x1, c.x2, c.y1);
			else if (ray_contact_line_segment({ c.x1, c.y1 }, { c.x1, c.y2 }, ray, t1, t2)) // first contact on left edge
				return first_contact_on_vertical_edge(ray.origin, t1, t2, c.y1, c.y2, c.x1);
			else if (ray_contact_line_segment({ c.x2, c.y1 }, { c.x2, c.y2 }, ray, t1, t2)) // first contact on right edge
				return first_contact_on_vertical_edge(ray.origin, t1, t2, c.y1, c.y2, c.x2);
			else // should be unreachable
				return { .hit = false };
		}

		// contact on left edge
		if (ray_contact_line_segment({ c.x1, c.y1 }, { c.x1, c.y2 }, ray, t1, t2))
		{
			if (ray.origin.x <= c.x1) // first contact on left edge
				return first_contact_on_vertical_edge(ray.origin, t1, t2, c.y1, c.y2, c.x1);
			else if (ray_contact_line_segment({ c.x2, c.y1 }, { c.x2, c.y2 }, ray, t1, t2)) // first contact on right edge
				return first_contact_on_vertical_edge(ray.origin, t1, t2, c.y1, c.y2, c.x2);
			else if (ray_contact_line_segment({ c.x1, c.y1 }, { c.x2, c.y1 }, ray, t1, t2)) // first contact on bottom edge
				return first_contact_on_horizontal_edge(ray.origin, t1, t2, c.x1, c.x2, c.y1);
			else if (ray_contact_line_segment({ c.x1, c.y2 }, { c.x2, c.y2 }, ray, t1, t2)) // first contact on top edge
				return first_contact_on_horizontal_edge(ray.origin, t1, t2, c.x1, c.x2, c.y2);
			else // should be unreachable
				return { .hit = false };
		}

		// contact on right edge
		if (ray_contact_line_segment({ c.x2, c.y1 }, { c.x2, c.y2 }, ray, t1, t2))
		{
			if (ray.origin.x >= c.x2) // first contact on right edge
				return first_contact_on_vertical_edge(ray.origin, t1, t2, c.y1, c.y2, c.x2);
			else if (ray_contact_line_segment({ c.x1, c.y1 }, { c.x1, c.y2 }, ray, t1, t2)) // first contact on left edge
				return first_contact_on_vertical_edge(ray.origin, t1, t2, c.y1, c.y2, c.x1);
			else if (ray_contact_line_segment({ c.x1, c.y1 }, { c.x2, c.y1 }, ray, t1, t2)) // first contact on bottom edge
				return first_contact_on_horizontal_edge(ray.origin, t1, t2, c.x1, c.x2, c.y1);
			else if (ray_contact_line_segment({ c.x1, c.y2 }, { c.x2, c.y2 }, ray, t1, t2)) // first contact on top edge
				return first_contact_on_horizontal_edge(ray.origin, t1, t2, c.x1, c.x2, c.y2);
			else // should be unreachable
				return { .hit = false };
		}

		return { .hit = false };
	}

	OverlapResult overlaps(const AABB& c1, const AABB& c2)
	{
		return c1.x1 <= c2.x2 && c2.x1 <= c1.x2 && c1.y1 <= c2.y2 && c2.y1 <= c1.y2;
	}

	CollisionResult collides(const AABB& c1, const AABB& c2)
	{
		CollisionResult info{};
		info.overlap = overlaps(c1, c2);

		if (info.overlap)
		{
			float dx1 = c1.x2 - c2.x1;
			float dx2 = c2.x2 - c1.x1;
			float dy1 = c1.y2 - c2.y1;
			float dy2 = c2.y2 - c1.y1;

			float overlapX = std::min(glm::abs(dx1), glm::abs(dx2));
			float overlapY = std::min(glm::abs(dy1), glm::abs(dy2));

			if (overlapX < overlapY)
			{
				info.penetration_depth = overlapX;
				info.unit_impulse.x = -(glm::abs(dx1) < glm::abs(dx2) ? glm::sign(dx1) : glm::sign(dx2));
				info.unit_impulse.y = 0.0f;
			}
			else
			{
				info.penetration_depth = overlapY;
				info.unit_impulse.x = 0.0f;
				info.unit_impulse.y = -(glm::abs(dy1) < glm::abs(dy2) ? glm::sign(dy1) : glm::sign(dy2));
			}
		}

		return info;
	}

	ContactResult contacts(const AABB& c1, const AABB& c2)
	{
		ContactResult info{};
		info.overlap = overlaps(c1, c2);

		if (info.overlap)
		{
			float dx1 = c1.x2 - c2.x1;
			float dx2 = c2.x2 - c1.x1;
			float dy1 = c1.y2 - c2.y1;
			float dy2 = c2.y2 - c1.y1;

			float overlapX = std::min(glm::abs(dx1), glm::abs(dx2));
			float overlapY = std::min(glm::abs(dy1), glm::abs(dy2));

			if (overlapX < overlapY)
			{
				if (glm::abs(dx1) < glm::abs(dx2))
				{
					info.active_feature.position.x = c1.x2;
					info.static_feature.position.x = c2.x1;

					info.active_feature.impulse.x = -overlapX;
				}
				else
				{
					info.active_feature.position.x = c1.x1;
					info.static_feature.position.x = c2.x2;

					info.active_feature.impulse.x = overlapX;
				}

				info.active_feature.position.y = 0.5f * (std::max(c1.y1, c2.y1) + std::min(c1.y2, c2.y2));
				info.static_feature.position.y = 0.5f * (std::max(c1.y1, c2.y1) + std::min(c1.y2, c2.y2));

				info.active_feature.impulse.y = 0.0f;
				info.static_feature.impulse.x = -info.active_feature.impulse.x;
				info.static_feature.impulse.y = 0.0f;
			}
			else
			{
				if (glm::abs(dy1) < glm::abs(dy2))
				{
					info.active_feature.position.y = c1.y2;
					info.static_feature.position.y = c2.y1;

					info.active_feature.impulse.y = -overlapY;
				}
				else
				{
					info.active_feature.position.y = c1.y1;
					info.static_feature.position.y = c2.y2;

					info.active_feature.impulse.y = overlapY;
				}

				info.active_feature.position.x = 0.5f * (std::max(c1.x1, c2.x1) + std::min(c1.x2, c2.x2));
				info.static_feature.position.x = 0.5f * (std::max(c1.x1, c2.x1) + std::min(c1.x2, c2.x2));

				info.active_feature.impulse.x = 0.0f;
				info.static_feature.impulse.x = 0.0f;
				info.static_feature.impulse.y = -info.active_feature.impulse.y;
			}
		}
		else // witness points - closest points on each AABB
		{
			if (c1.x1 > c2.x2) // c1 is right of c2
			{
				info.active_feature.position.x = c1.x1;
				info.static_feature.position.x = c2.x2;
			}
			else if (c2.x1 > c1.x2) // c2 is right of c1
			{
				info.active_feature.position.x = c1.x2;
				info.static_feature.position.x = c2.x1;
			}
			else // c1 and c2 overlap horizontally
				info.static_feature.position.x = info.active_feature.position.x = 0.5f * (std::max(c1.x1, c2.x1) + std::min(c1.x2, c2.x2));

			if (c1.y1 > c2.y2) // c1 is above c2
			{
				info.active_feature.position.y = c1.y1;
				info.static_feature.position.y = c2.y2;
			}
			else if (c2.y1 > c1.y2) // c2 is above c1
			{
				info.active_feature.position.y = c1.y2;
				info.static_feature.position.y = c2.y1;
			}
			else // c1 and c2 overlap vertically
				info.static_feature.position.y = info.active_feature.position.y = 0.5f * (std::max(c1.y1, c2.y1) + std::min(c1.y2, c2.y2));
		}

		return info;
	}

	OverlapResult overlaps(const Circle& c1, const AABB& c2)
	{
		// closest point on AABB to center of circle
		glm::vec2 closest_point = { glm::clamp(c1.center.x, c2.x1, c2.x2), glm::clamp(c1.center.y, c2.y1, c2.y2) };

		float dist_sqrd = math::mag_sqrd(c1.center - closest_point);
		return dist_sqrd <= c1.radius * c1.radius;
	}
	
	CollisionResult collides(const Circle& c1, const AABB& c2)
	{
		CollisionResult info{};

		// closest point on AABB to center of circle
		glm::vec2 closest_point = { glm::clamp(c1.center.x, c2.x1, c2.x2), glm::clamp(c1.center.y, c2.y1, c2.y2) };

		float dist_sqrd = math::mag_sqrd(c1.center - closest_point);
		info.overlap = dist_sqrd <= c1.radius * c1.radius;

		if (info.overlap)
		{
			if (near_zero(dist_sqrd)) // circle center is inside AABB
			{
				float dx1 = c1.center.x - c2.x1;
				float dx2 = c2.x2 - c1.center.x;
				float dy1 = c1.center.y - c2.y1;
				float dy2 = c2.y2 - c1.center.y;

				float dx = std::min(dx1, dx2);
				float dy = std::min(dy1, dy2);

				if (dx < dy)
				{
					info.penetration_depth = dx + c1.radius;
					info.unit_impulse.x = dx1 < dx2 ? -1.0f : 1.0f;
					info.unit_impulse.y = 0.0f;
				}
				else
				{
					info.penetration_depth = dy + c1.radius;
					info.unit_impulse.x = 0.0f;
					info.unit_impulse.y = dy1 < dy2 ? -1.0f : 1.0f;
				}
			}
			else // circle center is outside AABB
			{
				info.penetration_depth = c1.radius - glm::sqrt(dist_sqrd);
				info.unit_impulse = glm::normalize(c1.center - closest_point);
			}
		}

		return info;
	}
	
	ContactResult contacts(const Circle& c1, const AABB& c2)
	{
		ContactResult info{};

		// closest point on AABB to center of circle
		glm::vec2 closest_point = { glm::clamp(c1.center.x, c2.x1, c2.x2), glm::clamp(c1.center.y, c2.y1, c2.y2) };

		float dist_sqrd = math::mag_sqrd(c1.center - closest_point);
		info.overlap = dist_sqrd <= c1.radius * c1.radius;

		if (info.overlap)
		{
			if (near_zero(dist_sqrd)) // circle center is inside AABB
			{
				float dx1 = c1.center.x - c2.x1;
				float dx2 = c2.x2 - c1.center.x;
				float dy1 = c1.center.y - c2.y1;
				float dy2 = c2.y2 - c1.center.y;

				float dx = std::min(dx1, dx2);
				float dy = std::min(dy1, dy2);

				if (dx < dy)
				{
					float dirX = dx1 < dx2 ? 1.0f : -1.0f;
					info.active_feature.impulse = (dx + c1.radius) * glm::vec2{ -dirX, 0.0f };
					info.active_feature.position = c1.center + glm::vec2{ dirX * c1.radius, 0.0f };
					info.static_feature.position = glm::vec2{ dx1 < dx2 ? c2.x1 : c2.x2, c1.center.y };
				}
				else
				{
					float dirY = dy1 < dy2 ? 1.0f : -1.0f;
					info.active_feature.impulse = (dy + c1.radius) * glm::vec2{ 0.0f, -dirY };
					info.active_feature.position = c1.center + glm::vec2{ 0.0f, dirY * c1.radius };
					info.static_feature.position = glm::vec2{ c1.center.x, dy1 < dy2 ? c2.y1 : c2.y2 };
				}
				info.static_feature.impulse = -info.active_feature.impulse;
			}
			else // circle center is outside AABB
			{
				glm::vec2 displacement = glm::normalize(closest_point - c1.center);

				info.active_feature.impulse = (glm::sqrt(dist_sqrd) - c1.radius) * displacement;
				info.active_feature.position = c1.center + c1.radius * displacement;

				info.static_feature.impulse = -info.active_feature.impulse;
				info.static_feature.position = closest_point;
			}
		}

		return info;
	}
}
