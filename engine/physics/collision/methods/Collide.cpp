#include "Collide.h"

#include "physics/collision/methods/SAT.h"
#include "physics/collision/methods/GJK.h"
#include "physics/collision/methods/CircleMethods.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "core/types/Approximate.h"
#include "core/base/SimpleMath.h"
#include "core/base/Transforms.h"
#include "core/base/Constants.h"
#include "core/math/Solvers.h"

namespace oly::col2d
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
					proj_clip = nmax<float>();
				else if (below_zero(proj_direction))
					proj_clip = -nmax<float>();
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
					proj_clip = nmax<float>();
				else if (below_zero(proj_direction))
					proj_clip = -nmax<float>();
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
		glm::vec2 local = internal::CircleGlobalAccess::local_point(c, test);
		return math::mag_sqrd(local - c.center) <= c.radius * c.radius;
	}

	static OverlapResult ray_contact_circle(const Circle& c, const Ray& ray, float& t1, float& t2)
	{
		Ray local_ray{ .origin = internal::CircleGlobalAccess::local_point(c, ray.origin) };
		if (ray.clip == 0.0f)
		{
			local_ray.clip = 0.0f;
			local_ray.direction = internal::CircleGlobalAccess::local_direction(c, ray.direction);
		}
		else
		{
			glm::vec2 clip = internal::CircleGlobalAccess::local_direction(c, (glm::vec2)ray.direction * ray.clip);
			local_ray.clip = glm::length(clip);
			local_ray.direction = clip;
		}

		float cross = math::cross(local_ray.direction, c.center - local_ray.origin);
		float discriminant = c.radius * c.radius - cross * cross;
		if (discriminant < 0.0f)
			return false;

		float offset = local_ray.direction.dot(c.center - local_ray.origin);
		discriminant = glm::sqrt(discriminant);
		t1 = offset - discriminant;
		t2 = offset + discriminant;

		// no forward contact
		if (t2 < 0.0f)
			return false;

		// contact within clip
		bool contact = local_ray.clip == 0.0f || t1 <= local_ray.clip;
		float mult = math::inv_magnitude(internal::CircleGlobalAccess::local_direction(c, ray.direction));
		t1 *= mult;
		t2 *= mult;
		return contact;
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
		glm::vec2 local_contact = internal::CircleGlobalAccess::local_point(c, info.contact);
		info.normal = internal::CircleGlobalAccess::global_normal(c, local_contact - c.center);
		return info;
	}

	static float circle_penetration_depth(const Circle& c1, const Circle& c2, UnitVector2D& axis)
	{
		auto [min1, max1] = c1.projection_interval(axis);
		auto [min2, max2] = c2.projection_interval(axis);
		return sat::internal::sat(min1, max1, min2, max2, axis);
	}

	static bool circle_penetration_depth(const Circle& c1, const Circle& c2, UnitVector2D& minimizing_axis, float& depth)
	{
		float left = -glm::pi<float>(), right = glm::pi<float>();
		for (size_t i = 0; i < golden_iterations(1.0f / 360.0f) && right > left; ++i)
		{
			float m1 = right - (right - left) * inv_golden_ratio();
			float m2 = left + (right - left) * inv_golden_ratio();
			UnitVector2D a1(m1);
			float d1 = circle_penetration_depth(c1, c2, a1);
			if (d1 <= 0.0f)
				return false;
			UnitVector2D a2(m2);
			float d2 = circle_penetration_depth(c1, c2, a2);
			if (d2 <= 0.0f)
				return false;
			if (d1 < d2)
			{
				right = m2;
				minimizing_axis = a1;
				depth = d1;
			}
			else
			{
				left = m1;
				minimizing_axis = a2;
				depth = d2;
			}
		}
		return true;
	}

	OverlapResult overlaps(const Circle& c1, const Circle& c2)
	{
		if (internal::CircleGlobalAccess::has_no_global(c1) && internal::CircleGlobalAccess::has_no_global(c2))
		{
			float dist_sqrd = math::mag_sqrd(internal::CircleGlobalAccess::global_center(c2) - internal::CircleGlobalAccess::global_center(c1));
			float rsum = c1.radius + c2.radius;
			return dist_sqrd <= rsum * rsum;
		}
		else
		{
			if (!overlaps(internal::CircleGlobalAccess::bounding_circle(c1), internal::CircleGlobalAccess::bounding_circle(c2)))
				return false;

			if (point_hits(c1, internal::CircleGlobalAccess::global_point(c2, c2.center)))
				return true;
			if (point_hits(c2, internal::CircleGlobalAccess::global_point(c1, c1.center)))
				return true;

			UnitVector2D m;
			float d;
			return circle_penetration_depth(c1, c2, m, d);
		}
	}

	CollisionResult collides(const Circle& c1, const Circle& c2)
	{
		if (internal::CircleGlobalAccess::has_no_global(c1) && internal::CircleGlobalAccess::has_no_global(c2))
		{
			CollisionResult info{};
			info.overlap = overlaps(c1, c2);
			if (info.overlap)
			{
				glm::vec2 displacement = internal::CircleGlobalAccess::global_center(c1) - internal::CircleGlobalAccess::global_center(c2);
				info.penetration_depth = c1.radius + c2.radius - math::magnitude(displacement);
				if (!approx(internal::CircleGlobalAccess::global_center(c1), internal::CircleGlobalAccess::global_center(c2)))
					info.unit_impulse = UnitVector2D(displacement);
			}
			return info;
		}
		else
		{
			if (!overlaps(internal::CircleGlobalAccess::bounding_circle(c1), internal::CircleGlobalAccess::bounding_circle(c2)))
				return { .overlap = false };

			UnitVector2D axis;
			float depth;
			if (!circle_penetration_depth(c1, c2, axis, depth))
				return { .overlap = false };

			return CollisionResult{ .overlap = true, .penetration_depth = depth, .unit_impulse = axis };
		}
	}

	ContactResult contacts(const Circle& c1, const Circle& c2)
	{
		if (internal::CircleGlobalAccess::has_no_global(c1) && internal::CircleGlobalAccess::has_no_global(c2))
		{
			ContactResult info{};
			info.overlap = overlaps(c1, c2);

			UnitVector2D d(internal::CircleGlobalAccess::global_center(c2) - internal::CircleGlobalAccess::global_center(c1));
			info.active_feature.position = internal::CircleGlobalAccess::global_center(c1) + c1.radius * (glm::vec2)d;
			info.passive_feature.position = internal::CircleGlobalAccess::global_center(c2) - c2.radius * (glm::vec2)d;

			if (info.overlap)
			{
				info.active_feature.impulse = -(glm::vec2)d * (c1.radius + c2.radius
					- math::magnitude(internal::CircleGlobalAccess::global_center(c1) - internal::CircleGlobalAccess::global_center(c2)));
				info.passive_feature.impulse = -info.active_feature.impulse;
			}

			return info;
		}
		else
		{
			if (!overlaps(internal::CircleGlobalAccess::bounding_circle(c1), internal::CircleGlobalAccess::bounding_circle(c2)))
				return { .overlap = false };

			UnitVector2D axis;
			float depth;
			if (!circle_penetration_depth(c1, c2, axis, depth))
				return { .overlap = false };
			return standard_contact_result(c1, c2, axis, depth);
		}
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
		float max_entry = -nmax<float>();
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
					info.passive_feature.position.x = c2.x1;

					info.active_feature.impulse.x = -overlapX;
				}
				else
				{
					info.active_feature.position.x = c1.x1;
					info.passive_feature.position.x = c2.x2;

					info.active_feature.impulse.x = overlapX;
				}

				info.active_feature.position.y = 0.5f * (std::max(c1.y1, c2.y1) + std::min(c1.y2, c2.y2));
				info.passive_feature.position.y = 0.5f * (std::max(c1.y1, c2.y1) + std::min(c1.y2, c2.y2));

				info.active_feature.impulse.y = 0.0f;
				info.passive_feature.impulse.x = -info.active_feature.impulse.x;
				info.passive_feature.impulse.y = 0.0f;
			}
			else
			{
				if (glm::abs(dy1) < glm::abs(dy2))
				{
					info.active_feature.position.y = c1.y2;
					info.passive_feature.position.y = c2.y1;

					info.active_feature.impulse.y = -overlapY;
				}
				else
				{
					info.active_feature.position.y = c1.y1;
					info.passive_feature.position.y = c2.y2;

					info.active_feature.impulse.y = overlapY;
				}

				info.active_feature.position.x = 0.5f * (std::max(c1.x1, c2.x1) + std::min(c1.x2, c2.x2));
				info.passive_feature.position.x = 0.5f * (std::max(c1.x1, c2.x1) + std::min(c1.x2, c2.x2));

				info.active_feature.impulse.x = 0.0f;
				info.passive_feature.impulse.x = 0.0f;
				info.passive_feature.impulse.y = -info.active_feature.impulse.y;
			}
		}
		else // witness points - closest points on each AABB
		{
			if (c1.x1 > c2.x2) // c1 is right of c2
			{
				info.active_feature.position.x = c1.x1;
				info.passive_feature.position.x = c2.x2;
			}
			else if (c2.x1 > c1.x2) // c2 is right of c1
			{
				info.active_feature.position.x = c1.x2;
				info.passive_feature.position.x = c2.x1;
			}
			else // c1 and c2 overlap horizontally
				info.passive_feature.position.x = info.active_feature.position.x = 0.5f * (std::max(c1.x1, c2.x1) + std::min(c1.x2, c2.x2));

			if (c1.y1 > c2.y2) // c1 is above c2
			{
				info.active_feature.position.y = c1.y1;
				info.passive_feature.position.y = c2.y2;
			}
			else if (c2.y1 > c1.y2) // c2 is above c1
			{
				info.active_feature.position.y = c1.y2;
				info.passive_feature.position.y = c2.y1;
			}
			else // c1 and c2 overlap vertically
				info.passive_feature.position.y = info.active_feature.position.y = 0.5f * (std::max(c1.y1, c2.y1) + std::min(c1.y2, c2.y2));
		}

		return info;
	}

	OverlapResult point_hits(const OBB& c, glm::vec2 test)
	{
		glm::vec2 local = c.get_inv_rotation_matrix() * (test - c.center);
		return glm::abs(local.x) <= 0.5f * c.width && glm::abs(local.y) <= 0.5f * c.height;
	}

	OverlapResult ray_hits(const OBB& c, const Ray& ray)
	{
		auto proj = c.get_major_axis_projection_interval();
		if (!internal::ray_hits_slab(proj.first, proj.second, ray, c.get_major_axis()))
			return false;
		proj = c.get_minor_axis_projection_interval();
		return internal::ray_hits_slab(proj.first, proj.second, ray, c.get_minor_axis());
	}

	RaycastResult raycast(const OBB& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::EMBEDDED_ORIGIN, .contact = ray.origin };
		float max_entry = -nmax<float>();
		auto proj = c.get_major_axis_projection_interval();
		if (!internal::raycast_update_on_slab(proj.first, proj.second, ray, c.get_major_axis(), info, max_entry))
			return { .hit = RaycastResult::Hit::NO_HIT };
		proj = c.get_minor_axis_projection_interval();
		if (!internal::raycast_update_on_slab(proj.first, proj.second, ray, c.get_minor_axis(), info, max_entry))
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
		for (int i = 0; i < c.points().size(); ++i)
		{
			glm::vec2 curr = c.points()[i];
			glm::vec2 next = c.points()[(i + 1) % (int)c.points().size()];
			glm::vec2 prev = c.points()[unsigned_mod(i - 1, (int)c.points().size())];
			if (!math::in_convex_sector(prev - curr, next - curr, test - curr))
				return false;
		}
		return true;
	}

	OverlapResult ray_hits(const ConvexHull& c, const Ray& ray)
	{
		// origin is already in polygon
		if (point_hits(c, ray.origin))
			return true;

		for (size_t i = 0; i < c.points().size(); ++i)
		{
			// ray hits polygon edge
			if (ray_contact_line_segment(c.points()[i], c.points()[(i + 1) % c.points().size()], ray))
				return true;
		}
		return false;
	}

	RaycastResult raycast(const ConvexHull& c, const Ray& ray)
	{
		// origin is already in polygon
		if (point_hits(c, ray.origin))
			return { .hit = RaycastResult::Hit::EMBEDDED_ORIGIN };

		float closest_edge_distance = nmax<float>();;
		size_t closest_idx = -1;
		for (size_t i = 0; i < c.points().size(); ++i)
		{
			// ray hits polygon edge
			float t1, t2;
			if (ray_contact_line_segment(c.points()[i], c.points()[(i + 1) % c.points().size()], ray, t1, t2))
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
				.normal = -c.edge_normal(closest_idx)
			};
		}
		else
			return { .hit = RaycastResult::Hit::NO_HIT };
	}

	OverlapResult overlaps(const ConvexHull& c1, const ConvexHull& c2)
	{
		if (c1.points().size() + c2.points().size() >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::overlaps(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW)
					throw e;
			}
		}
		return sat::overlaps(c1, c2);
	}

	CollisionResult collides(const ConvexHull& c1, const ConvexHull& c2)
	{
		if (c1.points().size() + c2.points().size() >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::collides(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const ConvexHull& c1, const ConvexHull& c2)
	{
		if (c1.points().size() + c2.points().size() >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::contacts(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return sat::contacts(c1, c2);
	}

	OverlapResult overlaps(const Circle& c1, const AABB& c2)
	{
		if (internal::CircleGlobalAccess::has_no_global(c1))
		{
			glm::vec2 center = internal::CircleGlobalAccess::global_center(c1);
			// closest point in AABB to center of circle
			glm::vec2 closest_point = { glm::clamp(center.x, c2.x1, c2.x2), glm::clamp(center.y, c2.y1, c2.y2) };
			float dist_sqrd = math::mag_sqrd(center - closest_point);
			return dist_sqrd <= c1.radius * c1.radius;
		}
		else
			return internal::circle_overlaps_polygon(c1, c2.points());
	}
	
	CollisionResult collides(const Circle& c1, const AABB& c2)
	{
		if (internal::CircleGlobalAccess::has_no_global(c1))
		{
			CollisionResult info{};

			glm::vec2 center = internal::CircleGlobalAccess::global_center(c1);
			// closest point on AABB to center of circle
			glm::vec2 closest_point = { glm::clamp(center.x, c2.x1, c2.x2), glm::clamp(center.y, c2.y1, c2.y2) };

			float dist_sqrd = math::mag_sqrd(center - closest_point);
			info.overlap = dist_sqrd <= c1.radius * c1.radius;

			if (info.overlap)
			{
				if (near_zero(dist_sqrd)) // circle center is inside AABB
				{
					float dx1 = center.x - c2.x1;
					float dx2 = c2.x2 - center.x;
					float dy1 = center.y - c2.y1;
					float dy2 = c2.y2 - center.y;

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
					info.unit_impulse = center - closest_point;
				}
			}

			return info;
		}
		else
			return internal::circle_collides_polygon(c1, c2.points());
	}
	
	ContactResult contacts(const Circle& c1, const AABB& c2)
	{
		if (internal::CircleGlobalAccess::has_no_global(c1))
		{
			ContactResult info{};

			glm::vec2 center = internal::CircleGlobalAccess::global_center(c1);
			// closest point on AABB to center of circle
			glm::vec2 closest_point = { glm::clamp(center.x, c2.x1, c2.x2), glm::clamp(center.y, c2.y1, c2.y2) };

			float dist_sqrd = math::mag_sqrd(center - closest_point);
			info.overlap = dist_sqrd <= c1.radius * c1.radius;

			if (info.overlap)
			{
				if (near_zero(dist_sqrd)) // circle center is inside AABB
				{
					float dx1 = center.x - c2.x1;
					float dx2 = c2.x2 - center.x;
					float dy1 = center.y - c2.y1;
					float dy2 = c2.y2 - center.y;

					float dx = std::min(dx1, dx2);
					float dy = std::min(dy1, dy2);

					if (dx < dy)
					{
						float dirX = dx1 < dx2 ? 1.0f : -1.0f;
						info.active_feature.impulse = (dx + c1.radius) * glm::vec2{ -dirX, 0.0f };
						info.active_feature.position = center + glm::vec2{ dirX * c1.radius, 0.0f };
						info.passive_feature.position = glm::vec2{ dx1 < dx2 ? c2.x1 : c2.x2, center.y };
					}
					else
					{
						float dirY = dy1 < dy2 ? 1.0f : -1.0f;
						info.active_feature.impulse = (dy + c1.radius) * glm::vec2{ 0.0f, -dirY };
						info.active_feature.position = center + glm::vec2{ 0.0f, dirY * c1.radius };
						info.passive_feature.position = glm::vec2{ center.x, dy1 < dy2 ? c2.y1 : c2.y2 };
					}
					info.passive_feature.impulse = -info.active_feature.impulse;
				}
				else // circle center is outside AABB
				{
					UnitVector2D displacement(closest_point - center);

					info.active_feature.impulse = (glm::sqrt(dist_sqrd) - c1.radius) * (glm::vec2)displacement;
					info.active_feature.position = center + c1.radius * (glm::vec2)displacement;

					info.passive_feature.impulse = -info.active_feature.impulse;
					info.passive_feature.position = closest_point;
				}
			}

			return info;
		}
		else
			return internal::circle_contacts_polygon(c1, c2, c2.points());
	}

	OverlapResult overlaps(const Circle& c1, const OBB& c2)
	{
		if (internal::CircleGlobalAccess::has_no_global(c1))
		{
			// closest point in OBB to center of circle
			glm::mat2 inv_rot = glm::inverse(c2.get_rotation_matrix());
			glm::vec2 local_c1_center = inv_rot * internal::CircleGlobalAccess::global_center(c1);
			glm::vec2 local_c2_center = inv_rot * c2.center;
			glm::vec2 closest_point = { glm::clamp(local_c1_center.x, local_c2_center.x - 0.5f * c2.width, local_c2_center.x + 0.5f * c2.width),
				glm::clamp(local_c1_center.y, local_c2_center.y - 0.5f * c2.height, local_c2_center.y + 0.5f * c2.height) };
			float dist_sqrd = math::mag_sqrd(local_c1_center - closest_point);
			return dist_sqrd <= c1.radius * c1.radius;
		}
		else
			return internal::circle_overlaps_polygon(c1, c2.points());
	}

	CollisionResult collides(const Circle& c1, const OBB& c2)
	{
		if (internal::CircleGlobalAccess::has_no_global(c1))
		{
			CollisionResult info{};

			glm::vec2 center = internal::CircleGlobalAccess::global_center(c1);
			// closest point on OBB to center of circle
			glm::mat2 rot = c2.get_rotation_matrix();
			glm::mat2 inv_rot = glm::inverse(rot);
			glm::vec2 local_c1_center = inv_rot * center;
			glm::vec2 local_c2_center = inv_rot * c2.center;
			AABB b2{ .x1 = local_c2_center.x - 0.5f * c2.width, .x2 = local_c2_center.x + 0.5f * c2.width, .y1 = local_c2_center.y - 0.5f * c2.height, .y2 = local_c2_center.y + 0.5f * c2.height };
			glm::vec2 closest_point = { glm::clamp(local_c1_center.x, b2.x1, b2.x2), glm::clamp(local_c1_center.y, b2.y1, b2.y2) };
			float dist_sqrd = math::mag_sqrd(local_c1_center - closest_point);
			info.overlap = dist_sqrd <= c1.radius * c1.radius;

			if (info.overlap)
			{
				if (near_zero(dist_sqrd)) // circle center is inside OBB
				{
					float dx1 = local_c1_center.x - b2.x1;
					float dx2 = b2.x2 - center.x;
					float dy1 = local_c1_center.y - b2.y1;
					float dy2 = b2.y2 - center.y;

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
				else // circle center is outside OBB
				{
					info.penetration_depth = c1.radius - glm::sqrt(dist_sqrd);
					info.unit_impulse = local_c1_center - closest_point;
				}
			}

			glm::vec2 mtv = info.penetration_depth * (glm::vec2)info.unit_impulse;
			mtv = rot * mtv;
			info.penetration_depth = glm::length(mtv);
			info.unit_impulse = UnitVector2D(mtv);

			return info;
		}
		else
			return internal::circle_collides_polygon(c1, c2.points());
	}

	ContactResult contacts(const Circle& c1, const OBB& c2)
	{
		if (internal::CircleGlobalAccess::has_no_global(c1))
		{
			ContactResult info{};

			glm::vec2 center = internal::CircleGlobalAccess::global_center(c1);
			// closest point on OBB to center of circle
			glm::mat2 rot = c2.get_rotation_matrix();
			glm::mat2 inv_rot = glm::inverse(rot);
			glm::vec2 local_c1_center = inv_rot * center;
			glm::vec2 local_c2_center = inv_rot * c2.center;
			AABB b2{ .x1 = local_c2_center.x - 0.5f * c2.width, .x2 = local_c2_center.x + 0.5f * c2.width, .y1 = local_c2_center.y - 0.5f * c2.height, .y2 = local_c2_center.y + 0.5f * c2.height };
			glm::vec2 closest_point = { glm::clamp(local_c1_center.x, b2.x1, b2.x2), glm::clamp(local_c1_center.y, b2.y1, b2.y2) };
			float dist_sqrd = math::mag_sqrd(local_c1_center - closest_point);
			info.overlap = dist_sqrd <= c1.radius * c1.radius;

			if (info.overlap)
			{
				if (near_zero(dist_sqrd)) // circle center is inside OBB
				{
					float dx1 = local_c1_center.x - b2.x1;
					float dx2 = b2.x2 - center.x;
					float dy1 = local_c1_center.y - b2.y1;
					float dy2 = b2.y2 - center.y;

					float dx = std::min(dx1, dx2);
					float dy = std::min(dy1, dy2);

					if (dx < dy)
					{
						float dirX = dx1 < dx2 ? 1.0f : -1.0f;
						info.active_feature.impulse = (dx + c1.radius) * glm::vec2{ -dirX, 0.0f };
						info.active_feature.position = local_c1_center + glm::vec2{ dirX * c1.radius, 0.0f };
						info.passive_feature.position = glm::vec2{ dx1 < dx2 ? b2.x1 : b2.x2, local_c1_center.y };
					}
					else
					{
						float dirY = dy1 < dy2 ? 1.0f : -1.0f;
						info.active_feature.impulse = (dy + c1.radius) * glm::vec2{ 0.0f, -dirY };
						info.active_feature.position = local_c1_center + glm::vec2{ 0.0f, dirY * c1.radius };
						info.passive_feature.position = glm::vec2{ local_c1_center.x, dy1 < dy2 ? b2.y1 : b2.y2 };
					}
					info.passive_feature.impulse = -info.active_feature.impulse;
				}
				else // circle center is outside OBB
				{
					UnitVector2D displacement(closest_point - local_c1_center);

					info.active_feature.impulse = (glm::sqrt(dist_sqrd) - c1.radius) * (glm::vec2)displacement;
					info.active_feature.position = local_c1_center + c1.radius * (glm::vec2)displacement;

					info.passive_feature.impulse = -info.active_feature.impulse;
					info.passive_feature.position = closest_point;
				}
			}

			info.active_feature.impulse = rot * info.active_feature.impulse;
			info.active_feature.position = rot * info.active_feature.position;
			info.passive_feature.impulse = rot * info.passive_feature.impulse;
			info.passive_feature.position = rot * info.passive_feature.position;

			return info;
		}
		else
			return internal::circle_contacts_polygon(c1, c2, c2.points());
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
		if (1 + c2.points().size() >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::overlaps(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW)
					throw e;
			}
		}
		return internal::circle_overlaps_polygon(c1, c2.points());
	}

	CollisionResult collides(const Circle& c1, const ConvexHull& c2)
	{
		if (1 + c2.points().size() >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::collides(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return internal::circle_collides_polygon(c1, c2.points());
	}

	ContactResult contacts(const Circle& c1, const ConvexHull& c2)
	{
		if (1 + c2.points().size() >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::contacts(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return internal::circle_contacts_polygon(c1, c2, c2.points());
	}

	OverlapResult overlaps(const ConvexHull& c1, const AABB& c2)
	{
		if (c1.points().size() + 4 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::overlaps(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW)
					throw e;
			}
		}
		return sat::overlaps(c1, c2);
	}

	CollisionResult collides(const ConvexHull& c1, const AABB& c2)
	{
		if (c1.points().size() + 4 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::collides(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const ConvexHull& c1, const AABB& c2)
	{
		if (c1.points().size() + 4 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::contacts(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return sat::contacts(c1, c2);
	}

	OverlapResult overlaps(const ConvexHull& c1, const OBB& c2)
	{
		if (c1.points().size() + 4 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::overlaps(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW)
					throw e;
			}
		}
		return sat::overlaps(c1, c2);
	}
	
	CollisionResult collides(const ConvexHull& c1, const OBB& c2)
	{
		if (c1.points().size() + 4 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::collides(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return sat::collides(c1, c2);
	}

	ContactResult contacts(const ConvexHull& c1, const OBB& c2)
	{
		if (c1.points().size() + 4 >= gjk::VERTICES_THRESHOLD)
		{
			try
			{
				return gjk::contacts(c1, c2);
			}
			catch (Error e)
			{
				if (e.code != ErrorCode::GJK_OVERFLOW && e.code != ErrorCode::EPA_OVERFLOW)
					throw e;
			}
		}
		return sat::contacts(c1, c2);
	}

	OverlapResult point_hits(const ElementParam& c, glm::vec2 test)
	{
		return std::visit([test](auto&& c) { return point_hits(*c, test); }, c);
	}

	OverlapResult ray_hits(const ElementParam& c, const Ray& ray)
	{
		return std::visit([&ray](auto&& c) { return ray_hits(*c, ray); }, c);
	}

	RaycastResult raycast(const ElementParam& c, const Ray& ray)
	{
		return std::visit([&ray](auto&& c) { return raycast(*c, ray); }, c);
	}

	OverlapResult overlaps(const ElementParam& c1, const ElementParam& c2)
	{
		return std::visit([c2](auto&& c1) { return std::visit([c1](auto&& c2) { return overlaps(*c1, *c2); }, c2); }, c1);
	}

	CollisionResult collides(const ElementParam& c1, const ElementParam& c2)
	{
		return std::visit([c2](auto&& c1) { return std::visit([c1](auto&& c2) { return collides(*c1, *c2); }, c2); }, c1);
	}
	
	ContactResult contacts(const ElementParam& c1, const ElementParam& c2)
	{
		return std::visit([c2](auto&& c1) { return std::visit([c1](auto&& c2) { return contacts(*c1, *c2); }, c2); }, c1);
	}
}
