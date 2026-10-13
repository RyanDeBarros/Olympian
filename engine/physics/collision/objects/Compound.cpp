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

    TCompound::TCompound(std::vector<Element> elements)
        : compound_({ std::move(elements) })
    {
    }

    const Transform2D& TCompound::get_local() const
    {
        return transformer_.get_local();
    }
    
    Transform2D& TCompound::set_local()
    {
        return transformer_.set_local();
    }
    
    bool TCompound::is_dirty() const
    {
        return dirty_ || transformer_.dirty();
    }

    Transformer2DExposure<> TCompound::get_transformer() const
    {
        return transformer_;
    }
    
    Transformer2DExposure<ex::Full> TCompound::set_transformer()
    {
        return transformer_;
    }

    const Compound& TCompound::get_compound() const
    {
        return compound_;
    }

    Compound& TCompound::set_compound()
    {
        dirty_ = true;
        return compound_;
    }

    const std::vector<Element>& TCompound::get_baked() const
    {
        if (dirty_ || transformer_.flush())
            bake();

        return baked_;
    }

    Mask TCompound::mask() const
    {
        return compound_.mask;
    }

    Mask& TCompound::mask()
    {
        return compound_.mask;
    }

    Layer TCompound::layer() const
    {
        return compound_.layer;
    }

    Layer& TCompound::layer()
    {
        return compound_.layer;
    }

    const CompoundPerfParameters& TCompound::perf() const
    {
        return compound_.perf;
    }

    CompoundPerfParameters& TCompound::perf()
    {
        return compound_.perf;
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

    void TCompound::bake() const
    {
        baked_.resize(compound_.elements.size());
        glm::mat3 g = transformer_.global();
        for (size_t i = 0; i < baked_.size(); ++i)
            baked_[i] = compound_.elements[i].transformed(g);
        dirty_ = false;
    }

    namespace internal
    {
        const std::vector<Element>& element_array(const Compound& c)
        {
            return c.elements;
        }

        const std::vector<Element>& element_array(const TCompound& c)
        {
            return c.get_baked();
        }

        CompoundPerfParameters perf(const Compound& c)
        {
            return c.perf;
        }

        CompoundPerfParameters perf(const TCompound& c)
        {
            return c.perf();
        }
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
        RaycastResult info{ .hit = RaycastResult::Hit::NoHit };
        float closest_dist_sqrd = nmax<float>();
        for (const auto& element : c.elements)
        {
            RaycastResult res = raycast(element, ray);
            if (res.hit == RaycastResult::Hit::EmbeddedOrigin)
                return res;
            else if (res.hit == RaycastResult::Hit::TrueHit)
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

    OverlapResult overlaps(const Compound& c1, const Element& c2)
    {
        return internal::overlaps(c1, c2);
    }

    OverlapResult overlaps(const Element& c1, const Compound& c2)
    {
        return overlaps(c2, c1);
    }

    CollisionResult collides(const Compound& c1, const Element& c2)
    {
        return internal::collides(c1, c2);
    }

    CollisionResult collides(const Element& c1, const Compound& c2)
    {
        return collides(c2, c1).invert();
    }

    ContactResult contacts(const Compound& c1, const Element& c2)
    {
        return internal::contacts(c1, c2);
    }

    ContactResult contacts(const Element& c1, const Compound& c2)
    {
        return contacts(c2, c1).invert();
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

    OverlapResult overlaps(const TCompound& c1, const Element& c2)
    {
        return internal::overlaps(c1, c2);
    }

    OverlapResult overlaps(const Element& c1, const TCompound& c2)
    {
        return overlaps(c2, c1);
    }

    CollisionResult collides(const TCompound& c1, const Element& c2)
    {
        return internal::collides(c1, c2);
    }

    CollisionResult collides(const Element& c1, const TCompound& c2)
    {
        return collides(c2, c1).invert();
    }

    ContactResult contacts(const TCompound& c1, const Element& c2)
    {
        return internal::contacts(c1, c2);
    }

    ContactResult contacts(const Element& c1, const TCompound& c2)
    {
        return contacts(c2, c1).invert();
    }

    OverlapResult overlaps(const Compound& c1, const Compound& c2)
    {
        return (c1.mask & c2.layer) && internal::overlaps(c1, c2);
    }

    CollisionResult collides(const Compound& c1, const Compound& c2)
    {
        return (c1.mask & c2.layer) ? internal::collides(c1, c2) : CollisionResult{ .overlap = false };
    }

    ContactResult contacts(const Compound& c1, const Compound& c2)
    {
        return (c1.mask & c2.layer) ? internal::contacts(c1, c2) : ContactResult{ .overlap = false };
    }

    OverlapResult overlaps(const TCompound& c1, const TCompound& c2)
    {
        return (c1.mask() & c2.layer()) && internal::overlaps(c1, c2);
    }

    CollisionResult collides(const TCompound& c1, const TCompound& c2)
    {
        return (c1.mask() & c2.layer()) ? internal::collides(c1, c2) : CollisionResult{ .overlap = false };
    }

    ContactResult contacts(const TCompound& c1, const TCompound& c2)
    {
        return (c1.mask() & c2.layer()) ? internal::contacts(c1, c2) : ContactResult{ .overlap = false };
    }

    OverlapResult overlaps(const TCompound& c1, const Compound& c2)
    {
        return (c1.mask() & c2.layer) && internal::overlaps(c1, c2);
    }

    CollisionResult collides(const TCompound& c1, const Compound& c2)
    {
        return (c1.mask() & c2.layer) ? internal::collides(c1, c2) : CollisionResult{ .overlap = false };
    }

    ContactResult contacts(const TCompound& c1, const Compound& c2)
    {
        return (c1.mask() & c2.layer) ? internal::contacts(c1, c2) : ContactResult{ .overlap = false };
    }

    OverlapResult overlaps(const Compound& c1, const TCompound& c2)
    {
        return (c1.mask & c2.layer()) && internal::overlaps(c1, c2);
    }

    CollisionResult collides(const Compound& c1, const TCompound& c2)
    {
        return (c1.mask & c2.layer()) ? internal::collides(c1, c2) : CollisionResult{ .overlap = false };
    }

    ContactResult contacts(const Compound& c1, const TCompound& c2)
    {
        return (c1.mask & c2.layer()) ? internal::contacts(c1, c2) : ContactResult{ .overlap = false };
    }
}
