#pragma once

#include "physics/collision/scene/ColliderObject.h"

namespace oly::col2d::internal
{
	extern void load_luts();

	extern OverlapResult lut_overlaps(const ColliderObject&, const ColliderObject&);
	extern CollisionResult lut_collides(const ColliderObject&, const ColliderObject&);
	extern ContactResult lut_contacts(const ColliderObject&, const ColliderObject&);
	extern math::Rect2D lut_flush(const ColliderObject&);
	extern bool lut_is_dirty(const ColliderObject&);
}
