#include "LUT.h"

#include "physics/collision/scene/CObjID.h"

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
#define OLY_LUT_OVERLAPS(Enum1, Enum2, Class1, Class2) lut.overlaps[Enum1][Enum2] = [](const void* ptr1, const void* ptr2) { return overlaps(*static_cast<const Class1*>(ptr1), *static_cast<const Class2*>(ptr2)); };

#define OLY_LUT_OVERLAPS_LIST(Enum, Class)\
		OLY_LUT_OVERLAPS(Enum, CObjID::PRIMITIVE, Class, Primitive);\
		OLY_LUT_OVERLAPS(Enum, CObjID::TPRIMITIVE, Class, TPrimitive);\
		OLY_LUT_OVERLAPS(Enum, CObjID::COMPOUND, Class, Compound);\
		OLY_LUT_OVERLAPS(Enum, CObjID::TCOMPOUND, Class, TCompound);\
		OLY_LUT_OVERLAPS(Enum, CObjID::BVH_AABB, Class, BVH<AABB>);\
		OLY_LUT_OVERLAPS(Enum, CObjID::TBVH_AABB, Class, TBVH<AABB>);\
		OLY_LUT_OVERLAPS(Enum, CObjID::BVH_OBB, Class, BVH<OBB>);\
		OLY_LUT_OVERLAPS(Enum, CObjID::TBVH_OBB, Class, TBVH<OBB>);\
		OLY_LUT_OVERLAPS(Enum, CObjID::BVH_KDOP2, Class, BVH<KDOP2>);\
		OLY_LUT_OVERLAPS(Enum, CObjID::TBVH_KDOP2, Class, TBVH<KDOP2>);\
		OLY_LUT_OVERLAPS(Enum, CObjID::BVH_KDOP3, Class, BVH<KDOP3>);\
		OLY_LUT_OVERLAPS(Enum, CObjID::TBVH_KDOP3, Class, TBVH<KDOP3>);\
		OLY_LUT_OVERLAPS(Enum, CObjID::BVH_KDOP4, Class, BVH<KDOP4>);\
		OLY_LUT_OVERLAPS(Enum, CObjID::TBVH_KDOP4, Class, TBVH<KDOP4>);

		OLY_LUT_OVERLAPS_LIST(CObjID::PRIMITIVE, Primitive);
		OLY_LUT_OVERLAPS_LIST(CObjID::TPRIMITIVE, TPrimitive);
		OLY_LUT_OVERLAPS_LIST(CObjID::COMPOUND, Compound);
		OLY_LUT_OVERLAPS_LIST(CObjID::TCOMPOUND, TCompound);
		OLY_LUT_OVERLAPS_LIST(CObjID::BVH_AABB, BVH<AABB>);
		OLY_LUT_OVERLAPS_LIST(CObjID::TBVH_AABB, TBVH<AABB>);
		OLY_LUT_OVERLAPS_LIST(CObjID::BVH_OBB, BVH<OBB>);
		OLY_LUT_OVERLAPS_LIST(CObjID::TBVH_OBB, TBVH<OBB>);
		OLY_LUT_OVERLAPS_LIST(CObjID::BVH_KDOP2, BVH<KDOP2>);
		OLY_LUT_OVERLAPS_LIST(CObjID::TBVH_KDOP2, TBVH<KDOP2>);
		OLY_LUT_OVERLAPS_LIST(CObjID::BVH_KDOP3, BVH<KDOP3>);
		OLY_LUT_OVERLAPS_LIST(CObjID::TBVH_KDOP3, TBVH<KDOP3>);
		OLY_LUT_OVERLAPS_LIST(CObjID::BVH_KDOP4, BVH<KDOP4>);
		OLY_LUT_OVERLAPS_LIST(CObjID::TBVH_KDOP4, TBVH<KDOP4>);

#undef OLY_LUT_OVERLAPS_LIST
#undef OLY_LUT_OVERLAPS
	}

	static void load_collides()
	{
#define OLY_LUT_COLLIDES(Enum1, Enum2, Class1, Class2) lut.collides[Enum1][Enum2] = [](const void* ptr1, const void* ptr2) { return collides(*static_cast<const Class1*>(ptr1), *static_cast<const Class2*>(ptr2)); };

#define OLY_LUT_COLLIDES_LIST(Enum, Class)\
		OLY_LUT_COLLIDES(Enum, CObjID::PRIMITIVE, Class, Primitive);\
		OLY_LUT_COLLIDES(Enum, CObjID::TPRIMITIVE, Class, TPrimitive);\
		OLY_LUT_COLLIDES(Enum, CObjID::COMPOUND, Class, Compound);\
		OLY_LUT_COLLIDES(Enum, CObjID::TCOMPOUND, Class, TCompound);\
		OLY_LUT_COLLIDES(Enum, CObjID::BVH_AABB, Class, BVH<AABB>);\
		OLY_LUT_COLLIDES(Enum, CObjID::TBVH_AABB, Class, TBVH<AABB>);\
		OLY_LUT_COLLIDES(Enum, CObjID::BVH_OBB, Class, BVH<OBB>);\
		OLY_LUT_COLLIDES(Enum, CObjID::TBVH_OBB, Class, TBVH<OBB>);\
		OLY_LUT_COLLIDES(Enum, CObjID::BVH_KDOP2, Class, BVH<KDOP2>);\
		OLY_LUT_COLLIDES(Enum, CObjID::TBVH_KDOP2, Class, TBVH<KDOP2>);\
		OLY_LUT_COLLIDES(Enum, CObjID::BVH_KDOP3, Class, BVH<KDOP3>);\
		OLY_LUT_COLLIDES(Enum, CObjID::TBVH_KDOP3, Class, TBVH<KDOP3>);\
		OLY_LUT_COLLIDES(Enum, CObjID::BVH_KDOP4, Class, BVH<KDOP4>);\
		OLY_LUT_COLLIDES(Enum, CObjID::TBVH_KDOP4, Class, TBVH<KDOP4>);

		OLY_LUT_COLLIDES_LIST(CObjID::PRIMITIVE, Primitive);
		OLY_LUT_COLLIDES_LIST(CObjID::TPRIMITIVE, TPrimitive);
		OLY_LUT_COLLIDES_LIST(CObjID::COMPOUND, Compound);
		OLY_LUT_COLLIDES_LIST(CObjID::TCOMPOUND, TCompound);
		OLY_LUT_COLLIDES_LIST(CObjID::BVH_AABB, BVH<AABB>);
		OLY_LUT_COLLIDES_LIST(CObjID::TBVH_AABB, TBVH<AABB>);
		OLY_LUT_COLLIDES_LIST(CObjID::BVH_OBB, BVH<OBB>);
		OLY_LUT_COLLIDES_LIST(CObjID::TBVH_OBB, TBVH<OBB>);
		OLY_LUT_COLLIDES_LIST(CObjID::BVH_KDOP2, BVH<KDOP2>);
		OLY_LUT_COLLIDES_LIST(CObjID::TBVH_KDOP2, TBVH<KDOP2>);
		OLY_LUT_COLLIDES_LIST(CObjID::BVH_KDOP3, BVH<KDOP3>);
		OLY_LUT_COLLIDES_LIST(CObjID::TBVH_KDOP3, TBVH<KDOP3>);
		OLY_LUT_COLLIDES_LIST(CObjID::BVH_KDOP4, BVH<KDOP4>);
		OLY_LUT_COLLIDES_LIST(CObjID::TBVH_KDOP4, TBVH<KDOP4>);

#undef OLY_LUT_COLLIDES_LIST
#undef OLY_LUT_COLLIDES
	}

	static void load_contacts()
	{
#define OLY_LUT_CONTACTS(Enum1, Enum2, Class1, Class2) lut.contacts[Enum1][Enum2] = [](const void* ptr1, const void* ptr2) { return contacts(*static_cast<const Class1*>(ptr1), *static_cast<const Class2*>(ptr2)); };

#define OLY_LUT_CONTACTS_LIST(Enum, Class)\
		OLY_LUT_CONTACTS(Enum, CObjID::PRIMITIVE, Class, Primitive);\
		OLY_LUT_CONTACTS(Enum, CObjID::TPRIMITIVE, Class, TPrimitive);\
		OLY_LUT_CONTACTS(Enum, CObjID::COMPOUND, Class, Compound);\
		OLY_LUT_CONTACTS(Enum, CObjID::TCOMPOUND, Class, TCompound);\
		OLY_LUT_CONTACTS(Enum, CObjID::BVH_AABB, Class, BVH<AABB>);\
		OLY_LUT_CONTACTS(Enum, CObjID::TBVH_AABB, Class, TBVH<AABB>);\
		OLY_LUT_CONTACTS(Enum, CObjID::BVH_OBB, Class, BVH<OBB>);\
		OLY_LUT_CONTACTS(Enum, CObjID::TBVH_OBB, Class, TBVH<OBB>);\
		OLY_LUT_CONTACTS(Enum, CObjID::BVH_KDOP2, Class, BVH<KDOP2>);\
		OLY_LUT_CONTACTS(Enum, CObjID::TBVH_KDOP2, Class, TBVH<KDOP2>);\
		OLY_LUT_CONTACTS(Enum, CObjID::BVH_KDOP3, Class, BVH<KDOP3>);\
		OLY_LUT_CONTACTS(Enum, CObjID::TBVH_KDOP3, Class, TBVH<KDOP3>);\
		OLY_LUT_CONTACTS(Enum, CObjID::BVH_KDOP4, Class, BVH<KDOP4>);\
		OLY_LUT_CONTACTS(Enum, CObjID::TBVH_KDOP4, Class, TBVH<KDOP4>);

		OLY_LUT_CONTACTS_LIST(CObjID::PRIMITIVE, Primitive);
		OLY_LUT_CONTACTS_LIST(CObjID::TPRIMITIVE, TPrimitive);
		OLY_LUT_CONTACTS_LIST(CObjID::COMPOUND, Compound);
		OLY_LUT_CONTACTS_LIST(CObjID::TCOMPOUND, TCompound);
		OLY_LUT_CONTACTS_LIST(CObjID::BVH_AABB, BVH<AABB>);
		OLY_LUT_CONTACTS_LIST(CObjID::TBVH_AABB, TBVH<AABB>);
		OLY_LUT_CONTACTS_LIST(CObjID::BVH_OBB, BVH<OBB>);
		OLY_LUT_CONTACTS_LIST(CObjID::TBVH_OBB, TBVH<OBB>);
		OLY_LUT_CONTACTS_LIST(CObjID::BVH_KDOP2, BVH<KDOP2>);
		OLY_LUT_CONTACTS_LIST(CObjID::TBVH_KDOP2, TBVH<KDOP2>);
		OLY_LUT_CONTACTS_LIST(CObjID::BVH_KDOP3, BVH<KDOP3>);
		OLY_LUT_CONTACTS_LIST(CObjID::TBVH_KDOP3, TBVH<KDOP3>);
		OLY_LUT_CONTACTS_LIST(CObjID::BVH_KDOP4, BVH<KDOP4>);
		OLY_LUT_CONTACTS_LIST(CObjID::TBVH_KDOP4, TBVH<KDOP4>);

#undef OLY_LUT_CONTACTS_LIST
#undef OLY_LUT_CONTACTS
	}

	static void load_flush()
	{
		lut.flush[CObjID::PRIMITIVE] = [](const void* ptr) { return Wrap<AABB>{}(param(static_cast<const Primitive*>(ptr)->element)).rect(); };
		lut.flush[CObjID::TPRIMITIVE] = [](const void* ptr) { return Wrap<AABB>{}(param(static_cast<const TPrimitive*>(ptr)->get_baked())).rect(); };
		lut.flush[CObjID::COMPOUND] = [](const void* ptr) { return Wrap<AABB>{}(static_cast<const Compound*>(ptr)->elements.data(), static_cast<const Compound*>(ptr)->elements.size()).rect(); };
		lut.flush[CObjID::TCOMPOUND] = [](const void* ptr) { return Wrap<AABB>{}(static_cast<const TCompound*>(ptr)->get_baked().data(), static_cast<const TCompound*>(ptr)->get_baked().size()).rect(); };

#define OLY_LUT_FLUSH_BVH(Enum, Class) lut.flush[Enum] = [](const void* ptr) { return Wrap<AABB>{}(&static_cast<const Class*>(ptr)->root_shape()).rect(); };

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
#define OLY_LUT_IS_DIRTY_FALSE(Enum, Class) lut.is_dirty[Enum] = [](const void* ptr) { return false; };
#define OLY_LUT_IS_DIRTY_CHECK(Enum, Class) lut.is_dirty[Enum] = [](const void* ptr) { return static_cast<const Class*>(ptr)->is_dirty(); };

		OLY_LUT_IS_DIRTY_FALSE(CObjID::PRIMITIVE, Primitive);
		OLY_LUT_IS_DIRTY_CHECK(CObjID::TPRIMITIVE, TPrimitive);
		OLY_LUT_IS_DIRTY_FALSE(CObjID::COMPOUND, Compound);
		OLY_LUT_IS_DIRTY_CHECK(CObjID::TCOMPOUND, TCompound);
		OLY_LUT_IS_DIRTY_FALSE(CObjID::BVH_AABB, BVH<AABB>);
		OLY_LUT_IS_DIRTY_CHECK(CObjID::TBVH_AABB, TBVH<AABB>);
		OLY_LUT_IS_DIRTY_FALSE(CObjID::BVH_OBB, BVH<OBB>);
		OLY_LUT_IS_DIRTY_CHECK(CObjID::TBVH_OBB, TBVH<OBB>);
		OLY_LUT_IS_DIRTY_FALSE(CObjID::BVH_KDOP2, BVH<KDOP2>);
		OLY_LUT_IS_DIRTY_CHECK(CObjID::TBVH_KDOP2, TBVH<KDOP2>);
		OLY_LUT_IS_DIRTY_FALSE(CObjID::BVH_KDOP3, BVH<KDOP3>);
		OLY_LUT_IS_DIRTY_CHECK(CObjID::TBVH_KDOP3, TBVH<KDOP3>);
		OLY_LUT_IS_DIRTY_FALSE(CObjID::BVH_KDOP4, BVH<KDOP4>);
		OLY_LUT_IS_DIRTY_CHECK(CObjID::TBVH_KDOP4, TBVH<KDOP4>);

#undef OLY_LUT_IS_DIRTY_FALSE
#undef OLY_LUT_IS_DIRTY_CHECK
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
