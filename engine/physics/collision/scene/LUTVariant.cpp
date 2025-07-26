#include "LUTVariant.h"

namespace oly::col2d::internal
{
	using VariantFn = ColliderObjectVariant(*)(void*);
	using ConstVariantFn = ColliderObjectConstVariant(*)(const void*);

	struct LUTVariant
	{
		VariantFn variant[(size_t)CObjID::_COUNT];
		ConstVariantFn const_variant[(size_t)CObjID::_COUNT];
	} lut;

	static void load_variant_lut()
	{
#define OLY_LUT_VARIANT(Class) lut.variant[cobj_id_of<Class>] = [](void* ptr) -> ColliderObjectVariant { return static_cast<Class*>(ptr); };

		OLY_LUT_VARIANT(TPrimitive);
		OLY_LUT_VARIANT(TCompound);
		OLY_LUT_VARIANT(TBVH<AABB>);
		OLY_LUT_VARIANT(TBVH<OBB>);
		OLY_LUT_VARIANT(TBVH<KDOP2>);
		OLY_LUT_VARIANT(TBVH<KDOP3>);
		OLY_LUT_VARIANT(TBVH<KDOP4>);

#undef OLY_LUT_VARIANT
	}

	static void load_const_variant_lut()
	{
#define OLY_LUT_CONST_VARIANT(Class) lut.const_variant[cobj_id_of<Class>] = [](const void* ptr) -> ColliderObjectConstVariant { return static_cast<const Class*>(ptr); };

		OLY_LUT_CONST_VARIANT(TPrimitive);
		OLY_LUT_CONST_VARIANT(TCompound);
		OLY_LUT_CONST_VARIANT(TBVH<AABB>);
		OLY_LUT_CONST_VARIANT(TBVH<OBB>);
		OLY_LUT_CONST_VARIANT(TBVH<KDOP2>);
		OLY_LUT_CONST_VARIANT(TBVH<KDOP3>);
		OLY_LUT_CONST_VARIANT(TBVH<KDOP4>);

#undef OLY_LUT_CONST_VARIANT
	}

	void load_variant_luts()
	{
		load_variant_lut();
		load_const_variant_lut();
	}

	ColliderObjectConstVariant lut_variant(const ColliderObject& c)
	{
		return (lut.const_variant[c.id()])(c.raw_obj());
	}

	ColliderObjectVariant lut_variant(ColliderObject& c)
	{
		return (lut.variant[c.id()])(const_cast<void*>(c.raw_obj()));
	}
}
