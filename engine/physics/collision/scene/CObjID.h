#pragma once

#include "physics/collision/scene/LUT.h"

namespace oly::col2d::internal
{
	enum CObjID : unsigned int
	{
		PRIMITIVE,
		TPRIMITIVE,
		COMPOUND,
		TCOMPOUND,
		BVH_AABB,
		TBVH_AABB,
		BVH_OBB,
		TBVH_OBB,
		BVH_KDOP2,
		TBVH_KDOP2,
		BVH_KDOP3,
		TBVH_KDOP3,
		BVH_KDOP4,
		TBVH_KDOP4,
		_COUNT
	};

	template<>
	struct CObjIDTrait<Primitive>
	{
		static constexpr CObjID ID = CObjID::PRIMITIVE;
	};

	template<>
	struct CObjIDTrait<TPrimitive>
	{
		static constexpr CObjID ID = CObjID::TPRIMITIVE;
	};

	template<>
	struct CObjIDTrait<Compound>
	{
		static constexpr CObjID ID = CObjID::COMPOUND;
	};

	template<>
	struct CObjIDTrait<TCompound>
	{
		static constexpr CObjID ID = CObjID::TCOMPOUND;
	};

	template<>
	struct CObjIDTrait<BVH<AABB>>
	{
		static constexpr CObjID ID = CObjID::BVH_AABB;
	};

	template<>
	struct CObjIDTrait<TBVH<AABB>>
	{
		static constexpr CObjID ID = CObjID::TBVH_AABB;
	};

	template<>
	struct CObjIDTrait<BVH<OBB>>
	{
		static constexpr CObjID ID = CObjID::BVH_OBB;
	};

	template<>
	struct CObjIDTrait<TBVH<OBB>>
	{
		static constexpr CObjID ID = CObjID::TBVH_OBB;
	};

	template<>
	struct CObjIDTrait<BVH<KDOP2>>
	{
		static constexpr CObjID ID = CObjID::BVH_KDOP2;
	};

	template<>
	struct CObjIDTrait<TBVH<KDOP2>>
	{
		static constexpr CObjID ID = CObjID::TBVH_KDOP2;
	};

	template<>
	struct CObjIDTrait<BVH<KDOP3>>
	{
		static constexpr CObjID ID = CObjID::BVH_KDOP3;
	};

	template<>
	struct CObjIDTrait<TBVH<KDOP3>>
	{
		static constexpr CObjID ID = CObjID::TBVH_KDOP3;
	};

	template<>
	struct CObjIDTrait<BVH<KDOP4>>
	{
		static constexpr CObjID ID = CObjID::BVH_KDOP4;
	};

	template<>
	struct CObjIDTrait<TBVH<KDOP4>>
	{
		static constexpr CObjID ID = CObjID::TBVH_KDOP4;
	};
}
