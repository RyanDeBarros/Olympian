#pragma once

#include "physics/collision/scene/ColliderObject.h"

namespace oly::col2d::internal
{
	using ColliderObjectConstVariant = std::variant<
		const TPrimitive*,
		const TCompound*,
		const TBVH<AABB>*,
		const TBVH<OBB>*,
		const TBVH<KDOP2>*,
		const TBVH<KDOP3>*,
		const TBVH<KDOP4>*
	>;

	using ColliderObjectVariant = std::variant<
		TPrimitive*,
		TCompound*,
		TBVH<AABB>*,
		TBVH<OBB>*,
		TBVH<KDOP2>*,
		TBVH<KDOP3>*,
		TBVH<KDOP4>*
	>;

	extern void load_variant_luts();

	extern ColliderObjectConstVariant lut_variant(const ColliderObject&);
	extern ColliderObjectVariant lut_variant(ColliderObject&);
}
