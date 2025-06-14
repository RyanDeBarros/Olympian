#include "Compound.h"

#include "core/base/SimpleMath.h"

namespace oly::col2d
{
	OverlapResult point_hits(const Compound& c, glm::vec2 test)
	{
		for (const auto& element : c.elements)
		{
			if (point_hits(element, test))
				return true;
		}
		return false;
	}

	OverlapResult ray_hits(const Compound& c, const Ray& ray)
	{
		for (const auto& element : c.elements)
		{
			if (ray_hits(element, ray))
				return true;
		}
		return false;
	}

	RaycastResult raycast(const Compound& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::NO_HIT };
		float closest_dist_sqrd = nmax<float>();
		for (const auto& element : c.elements)
		{
			RaycastResult res = raycast(element, ray);
			if (res.hit == RaycastResult::Hit::EMBEDDED_ORIGIN)
				return res;
			else if (res.hit == RaycastResult::Hit::TRUE_HIT)
			{
				float dist_sqrd = math::mag_sqrd(res.contact - ray.origin);
				if (dist_sqrd < closest_dist_sqrd)
				{
					closest_dist_sqrd = dist_sqrd;
					info = res;
				}
			}
		}
		return info;
	}

	OverlapResult internal::overlaps(const Compound& c1, const Compound& c2)
	{
		for (const auto& e1 : c1.elements)
			for (const auto& e2 : c2.elements)
				if (overlaps(e1, e2))
					return true;
		return false;
	}

	CollisionResult internal::collides(const Compound& c1, const Compound& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& e1 : c1.elements)
		{
			for (const auto& e2 : c2.elements)
			{
				CollisionResult collision = collides(e1, e2);
				if (collision.overlap)
					collisions.push_back(collision);
			}
		}
		return greedy_collision(collisions);
	}

	ContactResult internal::contacts(const Compound& c1, const Compound& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& e1 : c1.elements)
		{
			for (const auto& e2 : c2.elements)
			{
				ContactResult contact = contacts(e1, e2);
				if (contact.overlap)
					cntcts.push_back(contact);
			}
		}
		return greedy_contact(cntcts);
	}

	OverlapResult overlaps(const Compound& c1, const Element& c2)
	{
		for (const auto& e1 : c1.elements)
		{
			if (overlaps(e1, c2))
				return true;
		}
		return false;
	}
	
	CollisionResult collides(const Compound& c1, const Element& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& e1 : c1.elements)
		{
			CollisionResult collision = collides(e1, c2);
			if (collision.overlap)
				collisions.push_back(collision);
		}
		return greedy_collision(collisions);
	}
	
	ContactResult contacts(const Compound& c1, const Element& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& e1 : c1.elements)
		{
			ContactResult contact = contacts(e1, c2);
			if (contact.overlap)
				cntcts.push_back(contact);
		}
		return greedy_contact(cntcts);
	}

	void TCompound::bake() const
	{
		baked.resize(compound.elements.size());
		glm::mat3 g = transformer.global();
		for (size_t i = 0; i < baked.size(); ++i)
			baked[i] = std::visit([&g](auto&& e) { return transform_element(e, g); }, compound.elements[i]);
		dirty = false;
	}

	OverlapResult point_hits(const TCompound& c, glm::vec2 test)
	{
		glm::vec2 local_test = transform_point(glm::inverse(c.global()), test);
		return point_hits(c.get_compound(), local_test);
	}
	
	OverlapResult ray_hits(const TCompound& c, const Ray& ray)
	{
		glm::mat3 m = glm::inverse(c.global());
		Ray local_ray = { .origin = transform_point(m, ray.origin) };
		if (ray.clip == 0.0f)
		{
			local_ray.direction = transform_direction(m, ray.direction);
			local_ray.clip = 0.0f;
		}
		else
		{
			glm::vec2 clip = transform_direction(m, ray.clip * (glm::vec2)ray.direction);
			local_ray.direction = clip;
			local_ray.clip = glm::length(clip);
		}
		return ray_hits(c.get_compound(), local_ray);
	}
	
	RaycastResult raycast(const TCompound& c, const Ray& ray)
	{
		glm::mat3 g = c.global();
		glm::mat3 m = glm::inverse(g);
		Ray local_ray = { .origin = transform_point(m, ray.origin) };
		if (ray.clip == 0.0f)
		{
			local_ray.direction = transform_direction(m, ray.direction);
			local_ray.clip = 0.0f;
		}
		else
		{
			glm::vec2 clip = transform_direction(m, ray.clip * (glm::vec2)ray.direction);
			local_ray.direction = clip;
			local_ray.clip = glm::length(clip);
		}
		RaycastResult result = raycast(c.get_compound(), local_ray);
		result.contact = transform_point(g, result.contact);
		result.normal = transform_normal(g, result.normal);
		return result;
	}
}
