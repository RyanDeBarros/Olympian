#include "Collide.h"

#include "physics/collision/abstract/methods/SAT.h"
#include "core/types/Approximate.h"
#include "core/base/SimpleMath.h"

namespace oly::acm2d
{
	namespace internal
	{
		OverlapResult ray_hits_slab(float min_proj, float max_proj, const Ray& ray, const UnitVector2D& axis)
		{
			float proj_origin = axis.dot(ray.origin);
			float proj_direction = axis.dot(ray.direction);

			float proj_clip;
			if (ray.clip == 0.0f)
			{
				if (near_zero(proj_direction))
					proj_clip = proj_origin;
				else if (above_zero(proj_direction))
					proj_clip = std::numeric_limits<float>::max();
				else if (below_zero(proj_direction))
					proj_clip = std::numeric_limits<float>::lowest();
			}
			else
				proj_clip = proj_origin + ray.clip * proj_direction;

			float proj_min = std::min(proj_origin, proj_clip);
			float proj_max = std::max(proj_origin, proj_clip);

			return proj_max >= min_proj && proj_min <= max_proj;
		}

		bool raycast_update_on_slab(float min_proj, float max_proj, const Ray& ray, const UnitVector2D& axis, RaycastResult& info, float& max_entry)
		{
			float proj_origin = axis.dot(ray.origin);
			float proj_direction = axis.dot(ray.direction);

			float proj_clip;
			if (ray.clip == 0.0f)
			{
				if (near_zero(proj_direction))
					proj_clip = proj_origin;
				else if (above_zero(proj_direction))
					proj_clip = std::numeric_limits<float>::max();
				else if (below_zero(proj_direction))
					proj_clip = std::numeric_limits<float>::lowest();
			}
			else
				proj_clip = proj_origin + ray.clip * proj_direction;

			float proj_min = std::min(proj_origin, proj_clip);
			float proj_max = std::max(proj_origin, proj_clip);

			if (proj_max < min_proj || proj_min > max_proj)
				return false;

			if (!near_zero(proj_direction)) // ray is not parallel
			{
				// boundary = proj_origin + t * proj_direction --> t = (boundary - proj_origin) / proj_direction
				if (proj_origin < min_proj)
				{
					float t = (min_proj - proj_origin) / proj_direction; // boundary == minimum
					if (above_zero(t) && t > max_entry)
					{
						max_entry = t;
						info.hit = RaycastResult::Hit::TRUE_HIT;
						info.normal = -axis;
					}
				}
				else if (proj_origin > max_proj)
				{
					float t = (max_proj - proj_origin) / proj_direction; // boundary == maximum
					if (above_zero(t) && t > max_entry)
					{
						max_entry = t;
						info.hit = RaycastResult::Hit::TRUE_HIT;
						info.normal = axis;
					}
				}
			}

			return true;
		}
	}

	OverlapResult point_hits(const Circle& c, glm::vec2 test)
	{
		return math::mag_sqrd(c.center - test) <= c.radius * c.radius;
	}

	static OverlapResult ray_contact_circle(const Circle& c, const Ray& ray, float& t1, float& t2)
	{
		float cross = math::cross(ray.direction, c.center - ray.origin);
		float discriminant = c.radius * c.radius - cross * cross;
		if (discriminant < 0.0f)
			return false;

		float offset = ray.direction.dot(c.center - ray.origin);
		discriminant = glm::sqrt(discriminant);
		t1 = offset - discriminant;
		t2 = offset + discriminant;

		// no forward contact
		if (t2 < 0.0f)
			return false;

		// contact within clip
		return ray.clip == 0.0f || t1 <= ray.clip;
	}

	static OverlapResult ray_contact_circle(const Circle& c, const Ray& ray)
	{
		float t1, t2;
		return ray_contact_circle(c, ray, t1, t2);
	}

	OverlapResult ray_hits(const Circle& c, const Ray& ray)
	{
		return ray_contact_circle(c, ray);
	}

	RaycastResult raycast(const Circle& c, const Ray& ray)
	{
		if (point_hits(c, ray.origin))
			return { .hit = RaycastResult::Hit::EMBEDDED_ORIGIN, .contact = ray.origin };

		float t1, t2;
		if (!ray_contact_circle(c, ray, t1, t2))
			return { .hit = RaycastResult::Hit::NO_HIT };

		RaycastResult info{ .hit = RaycastResult::Hit::TRUE_HIT };
		info.contact = std::max(t1, 0.0f) * (glm::vec2)ray.direction + ray.origin;
		info.normal = info.contact - c.center;
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
			if (!approx(c1.center, c2.center))
				info.unit_impulse = UnitVector2D(c1.center - c2.center);
		}
		return info;
	}

	ContactResult contacts(const Circle& c1, const Circle& c2)
	{
		ContactResult info{};
		info.overlap = overlaps(c1, c2);

		UnitVector2D d(c2.center - c1.center);
		info.active_feature.position = c1.center + c1.radius * (glm::vec2)d;
		info.static_feature.position = c2.center - c2.radius * (glm::vec2)d;

		if (info.overlap)
		{
			info.active_feature.impulse = (glm::vec2)UnitVector2D(c1.center - c2.center) * (c1.radius + c2.radius - math::magnitude(c1.center - c2.center));
			info.static_feature.impulse = -info.active_feature.impulse;
		}

		return info;
	}

	OverlapResult point_hits(const AABB& c, glm::vec2 test)
	{
		return test.x >= c.x1 && test.x <= c.x2 && test.y >= c.y1 && test.y <= c.y2;
	}

	static OverlapResult ray_contact_line_segment(glm::vec2 a, glm::vec2 b, const Ray& ray, float& t1, float& t2)
	{
		if (math::cross(ray.direction, a - b) != 0.0f) // ray and line segment are not parallel
		{
			// solution to linear system
			glm::vec2 q = glm::inverse(glm::mat2(ray.direction, a - b)) * (a - ray.origin);

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
			if (near_zero(ray.direction.y()))
			{
				float ts[2]{
					(a.x - ray.origin.x) / ray.direction.x(),
					(b.x - ray.origin.x) / ray.direction.x()
				};
				t1 = std::min(ts[0], ts[1]);
				t2 = std::max(ts[0], ts[1]);
			}
			else
			{
				float ts[2]{
					(a.y - ray.origin.y) / ray.direction.y(),
					(b.y - ray.origin.y) / ray.direction.y()
				};
				t1 = std::min(ts[0], ts[1]);
				t2 = std::max(ts[0], ts[1]);
			}

			// ray is pointing away from line segment
			if (t2 < 0.0f)
				return false;

			t1 = std::max(t1, 0.0f);

			// ray reaches line segment
			return ray.clip == 0.0f || t1 <= ray.clip;
		}
		else
			return false;
	}

	static OverlapResult ray_contact_line_segment(glm::vec2 a, glm::vec2 b, const Ray& ray)
	{
		float t1, t2;
		return ray_contact_line_segment(a, b, ray, t1, t2);
	}

	OverlapResult ray_hits(const AABB& c, const Ray& ray)
	{
		return internal::ray_hits_slab(c.x1, c.x2, ray, UnitVector2D::RIGHT) && internal::ray_hits_slab(c.y1, c.y2, ray, UnitVector2D::UP);
	}

	RaycastResult raycast(const AABB& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::EMBEDDED_ORIGIN, .contact = ray.origin };
		float max_entry = std::numeric_limits<float>::lowest();
		if (!internal::raycast_update_on_slab(c.x1, c.x2, ray, UnitVector2D::RIGHT, info, max_entry))
			return { .hit = RaycastResult::Hit::NO_HIT };
		if (!internal::raycast_update_on_slab(c.y1, c.y2, ray, UnitVector2D::UP, info, max_entry))
			return { .hit = RaycastResult::Hit::NO_HIT };
		if (info.hit == RaycastResult::Hit::TRUE_HIT)
			info.contact = ray.origin + max_entry * (glm::vec2)ray.direction;
		return info;
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
				info.unit_impulse = { -(glm::abs(dx1) < glm::abs(dx2) ? glm::sign(dx1) : glm::sign(dx2)), 0.0f };
			}
			else
			{
				info.penetration_depth = overlapY;
				info.unit_impulse = { 0.0f, -(glm::abs(dy1) < glm::abs(dy2) ? glm::sign(dy1) : glm::sign(dy2)) };
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

	OverlapResult point_hits(const OBB& c, glm::vec2 test)
	{
		glm::vec2 local = (c.get_rotation_matrix() * test - c.center);
		return glm::abs(local.x) <= c.width && glm::abs(local.y) <= c.height;
	}

	OverlapResult ray_hits(const OBB& c, const Ray& ray)
	{
		auto proj = c.get_axis_1_projection_interval();
		if (internal::ray_hits_slab(proj.first, proj.second, ray, c.get_axis_1()))
			return false;
		proj = c.get_axis_2_projection_interval();
		return internal::ray_hits_slab(proj.first, proj.second, ray, c.get_axis_2());
	}

	RaycastResult raycast(const OBB& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::EMBEDDED_ORIGIN, .contact = ray.origin };
		float max_entry = std::numeric_limits<float>::lowest();
		auto proj = c.get_axis_1_projection_interval();
		if (!internal::raycast_update_on_slab(proj.first, proj.second, ray, c.get_axis_1(), info, max_entry))
			return { .hit = RaycastResult::Hit::NO_HIT };
		proj = c.get_axis_2_projection_interval();
		if (!internal::raycast_update_on_slab(proj.first, proj.second, ray, c.get_axis_2(), info, max_entry))
			return { .hit = RaycastResult::Hit::NO_HIT };
		if (info.hit == RaycastResult::Hit::TRUE_HIT)
			info.contact = ray.origin + max_entry * (glm::vec2)ray.direction;
		return info;
	}

	OverlapResult overlaps(const OBB& c1, const OBB& c2)
	{
		return sat::overlaps(c1, c2);
	}

	CollisionResult collides(const OBB& c1, const OBB& c2)
	{
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const OBB& c1, const OBB& c2)
	{
		return sat::contacts(c1, c2);
	}

	OverlapResult point_hits(const ConvexHull& c, glm::vec2 test)
	{
		for (int i = 0; i < c.points.size(); ++i)
		{
			glm::vec2 u1 = c.points[unsigned_mod(i - 1, (int)c.points.size())] - c.points[i];
			glm::vec2 u2 = c.points[(i + 1) % c.points.size()] - c.points[i];
			if (!math::in_convex_sector(u1, u2, test))
				return false;
		}
		return true;
	}

	OverlapResult ray_hits(const ConvexHull& c, const Ray& ray)
	{
		// origin is already in polygon
		if (point_hits(c, ray.origin))
			return true;

		for (size_t i = 0; i < c.points.size(); ++i)
		{
			// ray hits polygon edge
			if (ray_contact_line_segment(c.points[i], c.points[(i + 1) % c.points.size()], ray))
				return true;
		}
		return false;
	}

	RaycastResult raycast(const ConvexHull& c, const Ray& ray)
	{
		// origin is already in polygon
		if (point_hits(c, ray.origin))
			return { .hit = RaycastResult::Hit::EMBEDDED_ORIGIN };

		float closest_edge_distance = std::numeric_limits<float>::max();;
		size_t closest_idx = -1;
		for (size_t i = 0; i < c.points.size(); ++i)
		{
			// ray hits polygon edge
			float t1, t2;
			if (ray_contact_line_segment(c.points[i], c.points[(i + 1) % c.points.size()], ray, t1, t2))
			{
				if (t1 < closest_edge_distance)
				{
					closest_edge_distance = t1;
					closest_idx = i;
				}
			}
		}

		if (closest_idx != size_t(-1))
		{
			return {
				.hit = RaycastResult::Hit::TRUE_HIT,
				.contact = ray.origin + closest_edge_distance * (glm::vec2)ray.direction,
				.normal = c.edge_normal(closest_idx)
			};
		}
		else
			return { .hit = RaycastResult::Hit::NO_HIT };
	}

	OverlapResult overlaps(const ConvexHull& c1, const ConvexHull& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::overlaps(c1, c2);
	}

	CollisionResult collides(const ConvexHull& c1, const ConvexHull& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const ConvexHull& c1, const ConvexHull& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::contacts(c1, c2);
	}

	OverlapResult point_hits(const Capsule& c, glm::vec2 test)
	{
		return point_hits(c.mid_obb(), test) || point_hits(c.upper_circle(), test) || point_hits(c.lower_circle(), test);
	}

	OverlapResult ray_hits(const Capsule& c, const Ray& ray)
	{
		return ray_hits(c.mid_obb(), ray) || ray_hits(c.upper_circle(), ray) || ray_hits(c.lower_circle(), ray);
	}

	RaycastResult raycast(const Capsule& c, const Ray& ray)
	{
		RaycastResult upper_circle_cast = raycast(c.upper_circle(), ray);
		if (upper_circle_cast.hit != RaycastResult::Hit::NO_HIT)
		{
			if (upper_circle_cast.normal.dot(c.vertical()) >= 0.0f) // on exposed semi-circle
				return upper_circle_cast;
			else
				return raycast(c.mid_obb(), ray);
		}
		RaycastResult lower_circle_cast = raycast(c.lower_circle(), ray);
		if (lower_circle_cast.hit != RaycastResult::Hit::NO_HIT)
		{
			if (lower_circle_cast.normal.dot(-c.vertical()) >= 0.0f) // on exposed semi-circle
				return lower_circle_cast;
			else
				return raycast(c.mid_obb(), ray);
		}
		return raycast(c.mid_obb(), ray);
	}

	OverlapResult overlaps(const Capsule& c1, const Capsule& c2)
	{
		auto c1a = c1.mid_obb();
		auto c1b = c1.lower_circle();
		auto c1c = c1.upper_circle();
		auto c2a = c2.mid_obb();
		auto c2b = c2.lower_circle();
		auto c2c = c2.upper_circle();

		return overlaps(c1a, c2a) || overlaps(c1a, c2b) || overlaps(c1a, c2c)
			|| overlaps(c1b, c2a) || overlaps(c1b, c2b) || overlaps(c1b, c2c)
			|| overlaps(c1c, c2a) || overlaps(c1c, c2b) || overlaps(c1c, c2c);
	}

	CollisionResult collides(const Capsule& c1, const Capsule& c2)
	{
		auto c1_obb = c1.mid_obb();
		auto c1_lc = c1.lower_circle();
		auto c1_uc = c1.upper_circle();
		auto c2_obb = c2.mid_obb();
		auto c2_lc = c2.lower_circle();
		auto c2_uc = c2.upper_circle();

		CollisionResult info;

		info = collides(c1_lc, c2_lc);
		if (info.overlap)
		{
			info.
		}

		// TODO




		return { .overlap = false };
	}

	ContactResult contacts(const Capsule& c1, const Capsule& c2)
	{
		// TODO
	}

	OverlapResult overlaps(const Circle& c1, const AABB& c2)
	{
		// closest point in AABB to center of circle
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
					info.unit_impulse = { dx1 < dx2 ? -1.0f : 1.0f, 0.0f };
				}
				else
				{
					info.penetration_depth = dy + c1.radius;
					info.unit_impulse = { 0.0f, dy1 < dy2 ? -1.0f : 1.0f };
				}
			}
			else // circle center is outside AABB
			{
				info.penetration_depth = c1.radius - glm::sqrt(dist_sqrd);
				info.unit_impulse = c1.center - closest_point;
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
				UnitVector2D displacement(closest_point - c1.center);

				info.active_feature.impulse = (glm::sqrt(dist_sqrd) - c1.radius) * (glm::vec2)displacement;
				info.active_feature.position = c1.center + c1.radius * (glm::vec2)displacement;

				info.static_feature.impulse = -info.active_feature.impulse;
				info.static_feature.position = closest_point;
			}
		}

		return info;
	}

	OverlapResult overlaps(const Circle& c1, const OBB& c2)
	{
		return sat::overlaps(c1, c2);
	}

	CollisionResult collides(const Circle& c1, const OBB& c2)
	{
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const Circle& c1, const OBB& c2)
	{
		return sat::contacts(c1, c2);
	}

	OverlapResult overlaps(const AABB& c1, const OBB& c2)
	{
		return sat::overlaps(c1, c2);
	}

	CollisionResult collides(const AABB& c1, const OBB& c2)
	{
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const AABB& c1, const OBB& c2)
	{
		return sat::contacts(c1, c2);
	}

	OverlapResult overlaps(const Circle& c1, const ConvexHull& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::overlaps(c1, c2);
	}

	CollisionResult collides(const Circle& c1, const ConvexHull& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const Circle& c1, const ConvexHull& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::contacts(c1, c2);
	}

	OverlapResult overlaps(const ConvexHull& c1, const AABB& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::overlaps(c1, c2);
	}

	CollisionResult collides(const ConvexHull& c1, const AABB& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const ConvexHull& c1, const AABB& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::contacts(c1, c2);
	}

	OverlapResult overlaps(const ConvexHull& c1, const OBB& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::overlaps(c1, c2);
	}
	
	CollisionResult collides(const ConvexHull& c1, const OBB& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const ConvexHull& c1, const OBB& c2)
	{
		// TODO only do if shapes have low enough degree. Otherwise, use GJK
		return sat::contacts(c1, c2);
	}
}
