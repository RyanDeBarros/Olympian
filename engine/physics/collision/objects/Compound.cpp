#include "Compound.h"

#include "core/base/SimpleMath.h"

namespace oly::col2d
{
	float Compound::projection_max(UnitVector2D axis) const
	{
		float proj_max = -nmax<float>();
		for (const Element& element : elements)
			proj_max = std::max(proj_max, element.projection_max(axis));
		return proj_max;
	}
	
	float Compound::projection_min(UnitVector2D axis) const
	{
		float proj_min = nmax<float>();
		for (const Element& element : elements)
			proj_min = std::min(proj_min, element.projection_min(axis));
		return proj_min;
	}

	float TCompound::projection_max(UnitVector2D axis) const
	{
		float proj_max = -nmax<float>();
		for (const Element& element : get_baked())
			proj_max = std::max(proj_max, element.projection_max(axis));
		return proj_max;
	}

	float TCompound::projection_min(UnitVector2D axis) const
	{
		float proj_min = nmax<float>();
		for (const Element& element : get_baked())
			proj_min = std::min(proj_min, element.projection_min(axis));
		return proj_min;
	}

	OverlapResult point_hits(const Compound& c, glm::vec2 test)
	{
		for (const auto& element : c.elements)
		{
			if (point_hits(element, test))
				return true;
		}
		return false;
	}

	OverlapResult ray_hits(const Compound& c, Ray ray)
	{
		for (const auto& element : c.elements)
		{
			if (ray_hits(element, ray))
				return true;
		}
		return false;
	}

	RaycastResult raycast(const Compound& c, Ray ray)
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

	void TCompound::bake() const
	{
		baked.resize(compound.elements.size());
		glm::mat3 g = transformer.global();
		for (size_t i = 0; i < baked.size(); ++i)
			baked[i] = compound.elements[i].transformed(g);
		dirty = false;
	}

	OverlapResult point_hits(const TCompound& c, glm::vec2 test)
	{
		glm::vec2 local_test = transform_point(glm::inverse(c.get_transformer().global()), test);
		return point_hits(c.get_compound(), local_test);
	}
	
	OverlapResult ray_hits(const TCompound& c, Ray ray)
	{
		glm::mat3 m = glm::inverse(c.get_transformer().global());
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
	
	RaycastResult raycast(const TCompound& c, Ray ray)
	{
		glm::mat3 g = c.get_transformer().global();
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
