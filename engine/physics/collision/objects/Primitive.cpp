#include "Primitive.h"

namespace oly::col2d
{
    float Primitive::projection_max(UnitVector2D axis) const
    {
        return element.projection_max(axis);
    }

    float Primitive::projection_min(UnitVector2D axis) const
    {
        return element.projection_min(axis);
    }

    fpair Primitive::projection_interval(UnitVector2D axis) const
    {
        return element.projection_interval(axis);
    }

    ContactManifold Primitive::deepest_manifold(UnitVector2D axis) const
    {
        return element.deepest_manifold(axis);
    }

    OverlapResult point_hits(const Primitive& c, glm::vec2 test)
    {
        return point_hits(c.element, test);
    }

    OverlapResult ray_hits(const Primitive& c, Ray ray)
    {
        return ray_hits(c.element, ray);
    }

    RaycastResult raycast(const Primitive& c, Ray ray)
    {
        return raycast(c.element, ray);
    }

    OverlapResult overlaps(const Primitive& c1, const Primitive& c2)
    {
        return (c1.mask & c2.layer) && overlaps(c1.element, c2.element);
    }

    CollisionResult collides(const Primitive& c1, const Primitive& c2)
    {
        return (c1.mask & c2.layer) ? collides(c1.element, c2.element) : CollisionResult{ .overlap = false };
    }

    ContactResult contacts(const Primitive& c1, const Primitive& c2)
    {
        return (c1.mask & c2.layer) ? contacts(c1.element, c2.element) : ContactResult{ .overlap = false };
    }

    TPrimitive::TPrimitive(Element element) noexcept
        : primitive_{ .element = std::move(element) }
    {
    }
    
    const Transform2D& TPrimitive::get_local() const
    {
        return transformer_.get_local();
    }
    
    Transform2D& TPrimitive::set_local()
    {
        return transformer_.set_local();
    }
    
    bool TPrimitive::is_dirty() const
    {
        return dirty_ || transformer_.dirty();
    }

    Transformer2DExposure<> TPrimitive::get_transformer() const
    {
        return transformer_;
    }
    
    Transformer2DExposure<ex::kFull> TPrimitive::set_transformer()
    {
        return transformer_;
    }

    const Primitive& TPrimitive::get_primitive() const
    {
        return primitive_;
    }
    
    Primitive& TPrimitive::set_primitive()
    {
        dirty_ = true;
        return primitive_;
    }
    
    const Element& TPrimitive::get_baked() const
    {
        if (dirty_ || transformer_.flush())
            bake();

        return baked_;
    }

    Mask TPrimitive::mask() const
    {
        return primitive_.mask;
    }
    
    Mask& TPrimitive::mask()
    {
        return primitive_.mask;
    }
    
    Layer TPrimitive::layer() const
    {
        return primitive_.layer;
    }
    
    Layer& TPrimitive::layer()
    {
        return primitive_.layer;
    }

    float TPrimitive::projection_max(UnitVector2D axis) const
    {
        return get_baked().projection_max(axis);
    }
    
    float TPrimitive::projection_min(UnitVector2D axis) const
    {
        return get_baked().projection_min(axis);
    }
    
    fpair TPrimitive::projection_interval(UnitVector2D axis) const
    {
        return get_baked().projection_interval(axis);
    }

    ContactManifold TPrimitive::deepest_manifold(UnitVector2D axis) const
    {
        return get_baked().deepest_manifold(axis);
    }

    void TPrimitive::bake() const
    {
        baked_ = primitive_.element.transformed(transformer_.global());
        dirty_ = false;
    }

    OverlapResult point_hits(const TPrimitive& c, glm::vec2 test)
    {
        return point_hits(c.get_baked(), test);
    }

    OverlapResult ray_hits(const TPrimitive& c, Ray ray)
    {
        return ray_hits(c.get_baked(), ray);
    }
    
    RaycastResult raycast(const TPrimitive& c, Ray ray)
    {
        return raycast(c.get_baked(), ray);
    }

    OverlapResult overlaps(const TPrimitive& c1, const TPrimitive& c2)
    {
        return (c1.mask() & c2.layer()) && overlaps(c1.get_baked(), c2.get_baked());
    }
    
    OverlapResult overlaps(const TPrimitive& c1, const Primitive& c2)
    {
        return (c1.mask() & c2.layer) && overlaps(c1.get_baked(), c2.element);
    }

    OverlapResult overlaps(const Primitive& c1, const TPrimitive& c2)
    {
        return (c1.mask & c2.layer()) && overlaps(c1.element, c2.get_baked());
    }

    CollisionResult collides(const TPrimitive& c1, const TPrimitive& c2)
    {
        return (c1.mask() & c2.layer())
            ? collides(c1.get_baked(), c2.get_baked())
            : CollisionResult{ .overlap = false };
    }
    
    CollisionResult collides(const TPrimitive& c1, const Primitive& c2)
    {
        return (c1.mask() & c2.layer)
            ? collides(c1.get_baked(), c2.element)
            : CollisionResult{ .overlap = false };
    }

    CollisionResult collides(const Primitive& c1, const TPrimitive& c2)
    {
        return (c1.mask & c2.layer())
            ? collides(c1.element, c2.get_baked())
            : CollisionResult{ .overlap = false };
    }

    ContactResult contacts(const TPrimitive& c1, const TPrimitive& c2)
    {
        return (c1.mask() & c2.layer())
            ? contacts(c1.get_baked(), c2.get_baked())
            : ContactResult{ .overlap = false };
    }
    
    ContactResult contacts(const TPrimitive& c1, const Primitive& c2)
    {
        return (c1.mask() & c2.layer)
            ? contacts(c1.get_baked(), c2.element)
            : ContactResult{ .overlap = false };
    }
    
    ContactResult contacts(const Primitive& c1, const TPrimitive& c2)
    {
        return (c1.mask & c2.layer())
            ? contacts(c1.element, c2.get_baked())
            : ContactResult{ .overlap = false };
    }

    OverlapResult overlaps(const Primitive& c1, const Element& c2)
    {
        return overlaps(c1.element, c2);
    }
    
    OverlapResult overlaps(const Element& c1, const Primitive& c2)
    {
        return overlaps(c1, c2.element);
    }
    
    CollisionResult collides(const Primitive& c1, const Element& c2)
    {
        return collides(c1.element, c2);
    }
    
    CollisionResult collides(const Element& c1, const Primitive& c2)
    {
        return collides(c1, c2.element);
    }
    
    ContactResult contacts(const Primitive& c1, const Element& c2)
    {
        return contacts(c1.element, c2);
    }
    
    ContactResult contacts(const Element& c1, const Primitive& c2)
    {
        return contacts(c1, c2.element);
    }

    OverlapResult overlaps(const TPrimitive& c1, const Element& c2)
    {
        return overlaps(c1.get_baked(), c2);
    }
    
    OverlapResult overlaps(const Element& c1, const TPrimitive& c2)
    {
        return overlaps(c1, c2.get_baked());
    }
    
    CollisionResult collides(const TPrimitive& c1, const Element& c2)
    {
        return collides(c1.get_baked(), c2);
    }
    
    CollisionResult collides(const Element& c1, const TPrimitive& c2)
    {
        return collides(c1, c2.get_baked());
    }
    
    ContactResult contacts(const TPrimitive& c1, const Element& c2)
    {
        return contacts(c1.get_baked(), c2);
    }
    
    ContactResult contacts(const Element& c1, const TPrimitive& c2)
    {
        return contacts(c1, c2.get_baked());
    }
}
