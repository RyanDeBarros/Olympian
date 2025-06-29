#include "Colliders.h"

namespace oly::col2d
{
	math::Rect2D PrimitiveCollider::quad_wrap() const
	{
		if (_d)
		{
			_d = false;
			wrap = internal::Wrap<AABB>{}(param(p.element)).rect();
		}
		return wrap;
	}

	math::Rect2D TPrimitiveCollider::quad_wrap() const
	{
		if (_d)
		{
			_d = false;
			wrap = internal::Wrap<AABB>{}(param(p.get_baked())).rect();
		}
		return wrap;
	}

	math::Rect2D CompoundCollider::quad_wrap() const
	{
		if (_d)
		{
			_d = false;
			wrap = internal::Wrap<AABB>{}(c.elements.data(), c.elements.size()).rect();
		}
		return wrap;
	}

	math::Rect2D TCompoundCollider::quad_wrap() const
	{
		if (_d)
		{
			_d = false;
			wrap = internal::Wrap<AABB>{}(c.get_baked().data(), c.get_baked().size()).rect();
		}
		return wrap;
	}
}
