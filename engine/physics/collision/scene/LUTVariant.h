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
		const TBVH<KDOP4>*,
		const TBVH<KDOP5>*,
		const TBVH<KDOP6>*,
		const TBVH<KDOP7>*,
		const TBVH<KDOP8>*
	>;

	using ColliderObjectVariant = std::variant<
		TPrimitive*,
		TCompound*,
		TBVH<AABB>*,
		TBVH<OBB>*,
		TBVH<KDOP2>*,
		TBVH<KDOP3>*,
		TBVH<KDOP4>*,
		TBVH<KDOP5>*,
		TBVH<KDOP6>*,
		TBVH<KDOP7>*,
		TBVH<KDOP8>*
	>;

	extern void load_variant_luts();

	extern ColliderObjectConstVariant lut_variant(const ColliderObject&);
	extern ColliderObjectVariant lut_variant(ColliderObject&);
}
