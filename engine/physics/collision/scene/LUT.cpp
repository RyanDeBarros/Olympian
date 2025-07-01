#include "LUT.h"

namespace oly::col2d::internal
{
	using OverlapsFn = OverlapResult(*)(const void*, const void*);
	using CollidesFn = CollisionResult(*)(const void*, const void*);
	using ContactsFn = ContactResult(*)(const void*, const void*);
	using FlushFn = math::Rect2D(*)(const void*);
	using IsDirtyFn = bool(*)(const void*);

	struct
	{
		OverlapsFn overlaps[CObjID::_COUNT][CObjID::_COUNT];
		CollidesFn collides[CObjID::_COUNT][CObjID::_COUNT];
		ContactsFn contacts[CObjID::_COUNT][CObjID::_COUNT];
		FlushFn flush[CObjID::_COUNT];
		IsDirtyFn is_dirty[CObjID::_COUNT];
	} static lut;

	static void load_overlaps()
	{
		// TODO
	}

	static void load_collides()
	{
		// TODO
	}

	static void load_contacts()
	{
		// TODO
	}

	static void load_flush()
	{
		lut.flush[CObjID::PRIMITIVE] = [](const void* ptr) {
			auto& c = *static_cast<const Primitive*>(ptr);
			return Wrap<AABB>{}(param(c.element)).rect();
			};
		lut.flush[CObjID::TPRIMITIVE] = [](const void* ptr) {
			auto& c = *static_cast<const TPrimitive*>(ptr);
			return Wrap<AABB>{}(param(c.get_baked())).rect();
			};
		lut.flush[CObjID::COMPOUND] = [](const void* ptr) {
			auto& c = *static_cast<const Compound*>(ptr);
			return Wrap<AABB>{}(c.elements.data(), c.elements.size()).rect();
			};
		lut.flush[CObjID::TCOMPOUND] = [](const void* ptr) {
			auto& c = *static_cast<const TCompound*>(ptr);
			return Wrap<AABB>{}(c.get_baked().data(), c.get_baked().size()).rect();
			};

#define OLY_LUT_FLUSH_BVH(Enum, Obj)\
		lut.flush[Enum] = [](const void* ptr) {\
			auto& c = *static_cast<const Obj*>(ptr);\
			return Wrap<AABB>{}(&c.root_shape()).rect();\
			};

		OLY_LUT_FLUSH_BVH(CObjID::BVH_AABB, BVH<AABB>);
		OLY_LUT_FLUSH_BVH(CObjID::TBVH_AABB, TBVH<AABB>);
		OLY_LUT_FLUSH_BVH(CObjID::BVH_OBB, BVH<OBB>);
		OLY_LUT_FLUSH_BVH(CObjID::TBVH_OBB, TBVH<OBB>);
		OLY_LUT_FLUSH_BVH(CObjID::BVH_KDOP2, BVH<KDOP2>);
		OLY_LUT_FLUSH_BVH(CObjID::TBVH_KDOP2, TBVH<KDOP2>);
		OLY_LUT_FLUSH_BVH(CObjID::BVH_KDOP3, BVH<KDOP3>);
		OLY_LUT_FLUSH_BVH(CObjID::TBVH_KDOP3, TBVH<KDOP3>);
		OLY_LUT_FLUSH_BVH(CObjID::BVH_KDOP4, BVH<KDOP4>);
		OLY_LUT_FLUSH_BVH(CObjID::TBVH_KDOP4, TBVH<KDOP4>);

#undef OLY_LUT_FLUSH_BVH
	}

	static void load_is_dirty()
	{
		// TODO
	}

	void load_luts()
	{
		load_overlaps();
		load_collides();
		load_contacts();
		load_flush();
		load_is_dirty();
	}

	OverlapResult lut_overlaps(const ColliderObject& c1, const ColliderObject& c2)
	{
		return (lut.overlaps[c1.id()][c2.id()])(c1.raw_obj(), c2.raw_obj());
	}
	
	CollisionResult lut_collides(const ColliderObject& c1, const ColliderObject& c2)
	{
		return (lut.collides[c1.id()][c2.id()])(c1.raw_obj(), c2.raw_obj());
	}
	
	ContactResult lut_contacts(const ColliderObject& c1, const ColliderObject& c2)
	{
		return (lut.contacts[c1.id()][c2.id()])(c1.raw_obj(), c2.raw_obj());
	}

	math::Rect2D lut_flush(const ColliderObject& c)
	{
		return (lut.flush[c.id()])(c.raw_obj());
	}
	
	bool lut_is_dirty(const ColliderObject& c)
	{
		return (lut.is_dirty[c.id()])(c.raw_obj());
	}
}
