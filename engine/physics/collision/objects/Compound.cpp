#include "Compound.h"

#include "core/base/SimpleMath.h"

namespace oly::col2d
{
	OverlapResult point_hits(const Compound& c, glm::vec2 test)
	{
		for (const auto& element : c.elements)
		{
			if (std::visit([test](auto&& element) { return point_hits(element, test); }, element))
				return true;
		}
		return false;
	}

	OverlapResult ray_hits(const Compound& c, const Ray& ray)
	{
		for (const auto& element : c.elements)
		{
			if (std::visit([&ray](auto&& element) { return ray_hits(element, ray); }, element))
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
			RaycastResult res = std::visit([&ray](auto&& element) { return raycast(element, ray); }, element);
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
		for (const auto& p1 : c1.elements)
		{
			if (std::visit([&c2](auto&& p1) {
				for (const auto& p2 : c2.elements)
					if (std::visit([&p1](auto&& p2) { return overlaps(p1, p2); }, p2))
						return true;
				return false;
			}, p1))
				return true;
		}
		return false;
	}

	CollisionResult internal::collides(const Compound& c1, const Compound& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.elements)
		{
			std::visit([&collisions, &c2](auto&& p1) {
				for (const auto& p2 : c2.elements)
				{
					CollisionResult collision = std::visit([&p1](auto&& p2) { return collides(p1, p2); }, p2);
					if (collision.overlap)
						collisions.push_back(collision);
				}
				}, p1);
		}
		return greedy_collision(collisions);
	}

	ContactResult internal::contacts(const Compound& c1, const Compound& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.elements)
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				for (const auto& p2 : c2.elements)
				{
					ContactResult contact = std::visit([&p1](auto&& p2) { return contacts(p1, p2); }, p2);
					if (contact.overlap)
						cntcts.push_back(contact);
				}
				}, p1);
		}
		return greedy_contact(cntcts);
	}

	OverlapResult overlaps(const Compound& c1, const Element& c2)
	{
		for (const auto& p1 : c1.elements)
			if (std::visit([&c2](auto&& p1) { return std::visit([&p1](auto&& c2) { return overlaps(p1, c2); }, c2); }, p1))
				return true;
		return false;
	}
	
	CollisionResult collides(const Compound& c1, const Element& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.elements)
		{
			std::visit([&collisions, &c2](auto&& p1) {
				CollisionResult collision = std::visit([&p1](auto&& c2) { return collides(p1, c2); }, c2);
				if (collision.overlap)
					collisions.push_back(collision);
				}, p1);
		}
		return greedy_collision(collisions);
	}
	
	ContactResult contacts(const Compound& c1, const Element& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.elements)
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				ContactResult contact = std::visit([&p1](auto&& c2) { return contacts(p1, c2); }, c2);
				if (contact.overlap)
					cntcts.push_back(contact);
				}, p1);
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
