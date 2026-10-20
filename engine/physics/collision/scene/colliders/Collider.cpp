#include "Collider.h"

#include "physics/collision/debugging/CoreShapes.h"

#include "physics/dynamics/bodies/RigidBody.h"

namespace oly::col2d
{
    Collider::Collider()
        : dispatch_handle_(*this)
    {
    }

    Collider::Collider(internal::ColliderObject&& obj)
        : obj_(std::move(obj))
        , dispatch_handle_(*this)
    {
    }

	Collider::Collider(const Collider& o)
		: obj_(o.obj_)
        , handles(*this, o.handles)
        , dirty_(o.dirty_)
        , dispatch_handle_(*this, o.dispatch_handle_)
        , quad_wrap_(o.quad_wrap_)
	{
	}

	Collider::Collider(Collider&& o) noexcept
		: obj_(std::move(o.obj_))
        , handles(*this, std::move(o.handles))
        , dirty_(o.dirty_)
        , rigid_body_(o.rigid_body_)
        , dispatch_handle_(*this, std::move(o.dispatch_handle_))
        , quad_wrap_(o.quad_wrap_)
	{
        o.rigid_body_ = nullptr;
	}

	Collider& Collider::operator=(const Collider& o)
	{
		if (this != &o)
		{
			obj_ = o.obj_;
			dirty_ = o.dirty_;
			dispatch_handle_ = o.dispatch_handle_;
			quad_wrap_ = o.quad_wrap_;
			handles = o.handles;
		}

		return *this;
	}

	Collider& Collider::operator=(Collider&& o) noexcept
	{
		if (this != &o)
		{
			obj_ = std::move(o.obj_);
			dirty_ = o.dirty_;
			dispatch_handle_ = std::move(o.dispatch_handle_);
			quad_wrap_ = o.quad_wrap_;
			handles = std::move(o.handles);

            rigid_body_ = o.rigid_body_;
            o.rigid_body_ = nullptr;
		}

		return *this;
	}

    void Collider::emplace(internal::ColliderObject&& obj)
    {
        dirty_ = true;
        this->obj_ = std::move(obj);
    }

    void Collider::flag()
    {
        dirty_ = true;
    }

    const Transform2D& Collider::get_local() const
    {
        return internal::lut_transformer(obj_).get_local();
    }
    
    Transform2D& Collider::set_local()
    {
        return internal::lut_transformer(obj_).set_local();
    }

    Transformer2DExposure<> Collider::get_transformer() const
    {
        return internal::lut_transformer(obj_);
    }

    Transformer2DExposure<ex::kFull> Collider::set_transformer()
    {
        return internal::lut_transformer(obj_);
    }

    Layer Collider::layer() const
    {
        return internal::lut_layer(obj_);
    }

    Layer& Collider::layer()
    {
        return internal::lut_layer(obj_);
    }

    Mask Collider::mask() const
    {
        return internal::lut_mask(obj_);
    }
    
    Mask& Collider::mask()
    {
        return internal::lut_mask(obj_);
    }

	bool Collider::one_way_blocks(const Collider& active) const
	{
		return !active.rigid_body_
            || !one_way_blocking
            || one_way_blocking->dot(active.rigid_body_->state().linear_velocity) < 0.0f;
	}

    OverlapResult Collider::point_hits(glm::vec2 test) const
    {
        return internal::lut_point_hits(obj_, test);
    }

    OverlapResult Collider::ray_hits(Ray ray) const
    {
        return internal::lut_ray_hits(obj_, ray);
    }
    
    RaycastResult Collider::raycast(Ray ray) const
    {
        return internal::lut_raycast(obj_, ray);
    }
    
    OverlapResult Collider::overlaps(const Collider& other) const
    {
        return internal::lut_overlaps(obj_, other.obj_);
    }

    CollisionResult Collider::collides(const Collider& other) const
    {
        return internal::lut_collides(obj_, other.obj_);
    }

    ContactResult Collider::contacts(const Collider& other) const
    {
        return internal::lut_contacts(obj_, other.obj_);
    }

    OverlapResult Collider::circle_cast_hits(const CircleCast& cast) const
    {
        return internal::lut_circle_cast_hits(obj_, cast);
    }

    OverlapResult Collider::rect_cast_hits(const RectCast& cast) const
    {
        return internal::lut_rect_cast_hits(obj_, cast);
    }

    debug::DebugOverlay Collider::create_debug_overlay(
        debug::DebugOverlayLayer& layer,
        glm::vec4 color,
        debug::DebugOverlay::PaintOptions paint_options) const
    {
        return internal::lut_create_debug_overlay(layer, obj_, color, paint_options);
    }
    
    void Collider::modify_debug_overlay(debug::DebugOverlay& overlay, size_t shape_index) const
    {
        internal::lut_modify_debug_overlay(overlay, obj_, shape_index);
    }

    bool Collider::is_dirty() const
    {
        return dirty_ || internal::lut_is_dirty(obj_);
    }

	void Collider::flush() const
	{
		if (!is_dirty())
			return;

		dirty_ = false;
		internal::lut_flush(obj_);
		handles.flush();
	}

    internal::ColliderObjectConstVariant Collider::get_object_variant() const
    {
        return internal::lut_variant(obj_);
    }

    internal::ColliderObjectVariant Collider::get_object_variant()
    {
        return internal::lut_variant(obj_);
    }
}
