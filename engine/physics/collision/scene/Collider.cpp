#include "Collider.h"

namespace oly::col2d
{
	Collider::Collider(const Collider& other)
		: obj(other.obj), handles(*this, other.handles), dirty(other.dirty), quad_wrap(other.quad_wrap)
	{
	}

	Collider::Collider(Collider&& other) noexcept
		: obj(std::move(other.obj)), handles(*this, std::move(other.handles)), dirty(other.dirty), quad_wrap(other.quad_wrap)
	{
	}

	Collider& Collider::operator=(const Collider& other)
	{
		if (this != &other)
		{
			obj = other.obj;
			dirty = other.dirty;
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
			quad_wrap = other.quad_wrap;
			handles = std::move(other.handles);
		}
		return *this;
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
