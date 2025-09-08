#include "Collider.h"

#include "physics/dynamics/bodies/RigidBody.h"

namespace oly::col2d
{
	Collider::Collider(const Collider& other)
		: obj(other.obj), handles(*this, other.handles), dirty(other.dirty), dispatch_handle(*this, other.dispatch_handle), quad_wrap(other.quad_wrap)
	{
	}

	Collider::Collider(Collider&& other) noexcept
		: obj(std::move(other.obj)), handles(*this, std::move(other.handles)), dirty(other.dirty), dispatch_handle(*this, std::move(other.dispatch_handle)), quad_wrap(other.quad_wrap)
	{
	}

	Collider& Collider::operator=(const Collider& other)
	{
		if (this != &other)
		{
			obj = other.obj;
			dirty = other.dirty;
			dispatch_handle = other.dispatch_handle;
			quad_wrap = other.quad_wrap;
			handles = other.handles;
		}
		return *this;
	}

	Collider& Collider::operator=(Collider&& other) noexcept
	{
		if (this != &other)
		{
			obj = std::move(other.obj);
			dirty = other.dirty;
			dispatch_handle = std::move(other.dispatch_handle);
			quad_wrap = other.quad_wrap;
			handles = std::move(other.handles);
		}
		return *this;
	}

	bool Collider::one_way_blocks(const Collider& active) const
	{
		return !active.rigid_body || !one_way_blocking || one_way_blocking->dot(active.rigid_body->state().linear_velocity) < 0.0f;
	}
	
	void Collider::flush() const
	{
		if (!is_dirty())
			return;

		dirty = false;
		internal::lut_flush(obj);
		handles.flush();
	}
}
