#include "Compound.h"

namespace oly::acm2d
{
	OverlapResult point_hits(const Compound& c, glm::vec2 test)
	{
		for (const auto& primitive : c.primitives)
		{
			if (std::visit([test](auto&& primitive) { return point_hits(primitive, test); }, primitive))
				return true;
		}
		return false;
	}

	OverlapResult ray_hits(const Compound& c, const Ray& ray)
	{
		for (const auto& primitive : c.primitives)
		{
			if (std::visit([&ray](auto&& primitive) { return ray_hits(primitive, ray); }, primitive))
				return true;
		}
		return false;
	}

	RaycastResult raycast(const Compound& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::NO_HIT };
		float closest_dist_sqrd = std::numeric_limits<float>::max();
		for (const auto& primitive : c.primitives)
		{
			RaycastResult res = std::visit([&ray](auto&& primitive) { return raycast(primitive, ray); }, primitive);
			if (res.hit == RaycastResult::Hit::EMBEDDED_ORIGIN)
				return res;
			else if (res.hit == RaycastResult::Hit::TRUE_HIT)
			{
				float dist_sqrd = math::mag_sqrd(res.contact - ray.origin);
				if (dist_sqrd < closest_dist_sqrd)
				{
					closest_dist_sqrd = dist_sqrd;
					info.hit = RaycastResult::Hit::TRUE_HIT;
					info.contact = res.contact;
					info.normal = res.normal;
				}
			}
		}
		return info;
	}

	OverlapResult overlaps(const Compound& c1, const Compound& c2)
	{
		for (const auto& p1 : c1.primitives)
		{
			if (std::visit([&c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					if (std::visit([&p1](auto&& p2) { return overlaps(p1, p2); }, p2))
						return true;
				}
				return false;
			}, p1))
				return true;
		}
		return false;
	}

	CollisionResult collides(const Compound& c1, const Compound& c2)
	{
		// TODO
	}

	ContactResult contacts(const Compound& c1, const Compound& c2)
	{
		// TODO
	}
}
