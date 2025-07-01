#include "LUT.h"

#include "physics/collision/scene/ColliderObject.h"

namespace oly::col2d::internal
{
	using PointHitsFn = OverlapResult(*)(const void*, glm::vec2);
	using RayHitsFn = OverlapResult(*)(const void*, const Ray&);
	using RaycastFn = RaycastResult(*)(const void*, const Ray&);
	using OverlapsFn = OverlapResult(*)(const void*, const void*);
	using CollidesFn = CollisionResult(*)(const void*, const void*);
	using ContactsFn = ContactResult(*)(const void*, const void*);
	using CircleCastHitsFn = OverlapResult(*)(const void*, const CircleCast&);
	using RectCastHitsFn = OverlapResult(*)(const void*, const RectCast&);
	
	using FlushFn = math::Rect2D(*)(const void*);
	using IsDirtyFn = bool(*)(const void*);
	using CollisionViewFn = debug::CollisionView(*)(const void*, glm::vec4);
	using UpdateViewFn = void(*)(debug::CollisionView&, const void*, glm::vec4);

	struct
	{
		PointHitsFn point_hits[CObjID::_COUNT];
		RayHitsFn ray_hits[CObjID::_COUNT];
		RaycastFn raycast[CObjID::_COUNT];
		OverlapsFn overlaps[CObjID::_COUNT][CObjID::_COUNT];
		CollidesFn collides[CObjID::_COUNT][CObjID::_COUNT];
		ContactsFn contacts[CObjID::_COUNT][CObjID::_COUNT];
		CircleCastHitsFn circle_cast_hits[CObjID::_COUNT];
		RectCastHitsFn rect_cast_hits[CObjID::_COUNT];

		FlushFn flush[CObjID::_COUNT];
		IsDirtyFn is_dirty[CObjID::_COUNT];
		CollisionViewFn collision_view[CObjID::_COUNT];
		UpdateViewFn update_view[CObjID::_COUNT];
	} static lut;

	static void load_point_hits()
	{
#define OLY_LUT_POINT_HITS(Class) lut.point_hits[CObjIDTrait<Class>::ID] = [](const void* ptr, glm::vec2 test) { return point_hits(*static_cast<const Class*>(ptr), test); };

		OLY_LUT_POINT_HITS(Primitive);
		OLY_LUT_POINT_HITS(TPrimitive);
		OLY_LUT_POINT_HITS(Compound);
		OLY_LUT_POINT_HITS(TCompound);
		OLY_LUT_POINT_HITS(BVH<AABB>);
		OLY_LUT_POINT_HITS(TBVH<AABB>);
		OLY_LUT_POINT_HITS(BVH<OBB>);
		OLY_LUT_POINT_HITS(TBVH<OBB>);
		OLY_LUT_POINT_HITS(BVH<KDOP2>);
		OLY_LUT_POINT_HITS(TBVH<KDOP2>);
		OLY_LUT_POINT_HITS(BVH<KDOP3>);
		OLY_LUT_POINT_HITS(TBVH<KDOP3>);
		OLY_LUT_POINT_HITS(BVH<KDOP4>);
		OLY_LUT_POINT_HITS(TBVH<KDOP4>);

#undef OLY_LUT_POINT_HITS
	}

	static void load_ray_hits()
	{
#define OLY_LUT_RAY_HITS(Class) lut.ray_hits[CObjIDTrait<Class>::ID] = [](const void* ptr, const Ray& ray) { return ray_hits(*static_cast<const Class*>(ptr), ray); };

		OLY_LUT_RAY_HITS(Primitive);
		OLY_LUT_RAY_HITS(TPrimitive);
		OLY_LUT_RAY_HITS(Compound);
		OLY_LUT_RAY_HITS(TCompound);
		OLY_LUT_RAY_HITS(BVH<AABB>);
		OLY_LUT_RAY_HITS(TBVH<AABB>);
		OLY_LUT_RAY_HITS(BVH<OBB>);
		OLY_LUT_RAY_HITS(TBVH<OBB>);
		OLY_LUT_RAY_HITS(BVH<KDOP2>);
		OLY_LUT_RAY_HITS(TBVH<KDOP2>);
		OLY_LUT_RAY_HITS(BVH<KDOP3>);
		OLY_LUT_RAY_HITS(TBVH<KDOP3>);
		OLY_LUT_RAY_HITS(BVH<KDOP4>);
		OLY_LUT_RAY_HITS(TBVH<KDOP4>);

#undef OLY_LUT_RAY_HITS
	}

	static void load_raycast()
	{
#define OLY_LUT_RAYCAST(Class) lut.raycast[CObjIDTrait<Class>::ID] = [](const void* ptr, const Ray& ray) { return raycast(*static_cast<const Class*>(ptr), ray); };

		OLY_LUT_RAYCAST(Primitive);
		OLY_LUT_RAYCAST(TPrimitive);
		OLY_LUT_RAYCAST(Compound);
		OLY_LUT_RAYCAST(TCompound);
		OLY_LUT_RAYCAST(BVH<AABB>);
		OLY_LUT_RAYCAST(TBVH<AABB>);
		OLY_LUT_RAYCAST(BVH<OBB>);
		OLY_LUT_RAYCAST(TBVH<OBB>);
		OLY_LUT_RAYCAST(BVH<KDOP2>);
		OLY_LUT_RAYCAST(TBVH<KDOP2>);
		OLY_LUT_RAYCAST(BVH<KDOP3>);
		OLY_LUT_RAYCAST(TBVH<KDOP3>);
		OLY_LUT_RAYCAST(BVH<KDOP4>);
		OLY_LUT_RAYCAST(TBVH<KDOP4>);

#undef OLY_LUT_RAYCAST
	}

	static void load_overlaps()
	{
#define OLY_LUT_OVERLAPS(Class1, Class2) lut.overlaps[CObjIDTrait<Class1>::ID][CObjIDTrait<Class2>::ID] = [](const void* ptr1, const void* ptr2)\
			{ return overlaps(*static_cast<const Class1*>(ptr1), *static_cast<const Class2*>(ptr2)); };

#define OLY_LUT_OVERLAPS_LIST(Class)\
		OLY_LUT_OVERLAPS(Class, Primitive);\
		OLY_LUT_OVERLAPS(Class, TPrimitive);\
		OLY_LUT_OVERLAPS(Class, Compound);\
		OLY_LUT_OVERLAPS(Class, TCompound);\
		OLY_LUT_OVERLAPS(Class, BVH<AABB>);\
		OLY_LUT_OVERLAPS(Class, TBVH<AABB>);\
		OLY_LUT_OVERLAPS(Class, BVH<OBB>);\
		OLY_LUT_OVERLAPS(Class, TBVH<OBB>);\
		OLY_LUT_OVERLAPS(Class, BVH<KDOP2>);\
		OLY_LUT_OVERLAPS(Class, TBVH<KDOP2>);\
		OLY_LUT_OVERLAPS(Class, BVH<KDOP3>);\
		OLY_LUT_OVERLAPS(Class, TBVH<KDOP3>);\
		OLY_LUT_OVERLAPS(Class, BVH<KDOP4>);\
		OLY_LUT_OVERLAPS(Class, TBVH<KDOP4>);

		OLY_LUT_OVERLAPS_LIST(Primitive);
		OLY_LUT_OVERLAPS_LIST(TPrimitive);
		OLY_LUT_OVERLAPS_LIST(Compound);
		OLY_LUT_OVERLAPS_LIST(TCompound);
		OLY_LUT_OVERLAPS_LIST(BVH<AABB>);
		OLY_LUT_OVERLAPS_LIST(TBVH<AABB>);
		OLY_LUT_OVERLAPS_LIST(BVH<OBB>);
		OLY_LUT_OVERLAPS_LIST(TBVH<OBB>);
		OLY_LUT_OVERLAPS_LIST(BVH<KDOP2>);
		OLY_LUT_OVERLAPS_LIST(TBVH<KDOP2>);
		OLY_LUT_OVERLAPS_LIST(BVH<KDOP3>);
		OLY_LUT_OVERLAPS_LIST(TBVH<KDOP3>);
		OLY_LUT_OVERLAPS_LIST(BVH<KDOP4>);
		OLY_LUT_OVERLAPS_LIST(TBVH<KDOP4>);

#undef OLY_LUT_OVERLAPS_LIST
#undef OLY_LUT_OVERLAPS
	}

	static void load_collides()
	{
#define OLY_LUT_COLLIDES(Class1, Class2) lut.collides[CObjIDTrait<Class1>::ID][CObjIDTrait<Class2>::ID] = [](const void* ptr1, const void* ptr2)\
			{ return collides(*static_cast<const Class1*>(ptr1), *static_cast<const Class2*>(ptr2)); };

#define OLY_LUT_COLLIDES_LIST(Class)\
		OLY_LUT_COLLIDES(Class, Primitive);\
		OLY_LUT_COLLIDES(Class, TPrimitive);\
		OLY_LUT_COLLIDES(Class, Compound);\
		OLY_LUT_COLLIDES(Class, TCompound);\
		OLY_LUT_COLLIDES(Class, BVH<AABB>);\
		OLY_LUT_COLLIDES(Class, TBVH<AABB>);\
		OLY_LUT_COLLIDES(Class, BVH<OBB>);\
		OLY_LUT_COLLIDES(Class, TBVH<OBB>);\
		OLY_LUT_COLLIDES(Class, BVH<KDOP2>);\
		OLY_LUT_COLLIDES(Class, TBVH<KDOP2>);\
		OLY_LUT_COLLIDES(Class, BVH<KDOP3>);\
		OLY_LUT_COLLIDES(Class, TBVH<KDOP3>);\
		OLY_LUT_COLLIDES(Class, BVH<KDOP4>);\
		OLY_LUT_COLLIDES(Class, TBVH<KDOP4>);

		OLY_LUT_COLLIDES_LIST(Primitive);
		OLY_LUT_COLLIDES_LIST(TPrimitive);
		OLY_LUT_COLLIDES_LIST(Compound);
		OLY_LUT_COLLIDES_LIST(TCompound);
		OLY_LUT_COLLIDES_LIST(BVH<AABB>);
		OLY_LUT_COLLIDES_LIST(TBVH<AABB>);
		OLY_LUT_COLLIDES_LIST(BVH<OBB>);
		OLY_LUT_COLLIDES_LIST(TBVH<OBB>);
		OLY_LUT_COLLIDES_LIST(BVH<KDOP2>);
		OLY_LUT_COLLIDES_LIST(TBVH<KDOP2>);
		OLY_LUT_COLLIDES_LIST(BVH<KDOP3>);
		OLY_LUT_COLLIDES_LIST(TBVH<KDOP3>);
		OLY_LUT_COLLIDES_LIST(BVH<KDOP4>);
		OLY_LUT_COLLIDES_LIST(TBVH<KDOP4>);

#undef OLY_LUT_COLLIDES_LIST
#undef OLY_LUT_COLLIDES
	}

	static void load_contacts()
	{
#define OLY_LUT_CONTACTS(Class1, Class2) lut.contacts[CObjIDTrait<Class1>::ID][CObjIDTrait<Class2>::ID] = [](const void* ptr1, const void* ptr2)\
			{ return contacts(*static_cast<const Class1*>(ptr1), *static_cast<const Class2*>(ptr2)); };

#define OLY_LUT_CONTACTS_LIST(Class)\
		OLY_LUT_CONTACTS(Class, Primitive);\
		OLY_LUT_CONTACTS(Class, TPrimitive);\
		OLY_LUT_CONTACTS(Class, Compound);\
		OLY_LUT_CONTACTS(Class, TCompound);\
		OLY_LUT_CONTACTS(Class, BVH<AABB>);\
		OLY_LUT_CONTACTS(Class, TBVH<AABB>);\
		OLY_LUT_CONTACTS(Class, BVH<OBB>);\
		OLY_LUT_CONTACTS(Class, TBVH<OBB>);\
		OLY_LUT_CONTACTS(Class, BVH<KDOP2>);\
		OLY_LUT_CONTACTS(Class, TBVH<KDOP2>);\
		OLY_LUT_CONTACTS(Class, BVH<KDOP3>);\
		OLY_LUT_CONTACTS(Class, TBVH<KDOP3>);\
		OLY_LUT_CONTACTS(Class, BVH<KDOP4>);\
		OLY_LUT_CONTACTS(Class, TBVH<KDOP4>);

		OLY_LUT_CONTACTS_LIST(Primitive);
		OLY_LUT_CONTACTS_LIST(TPrimitive);
		OLY_LUT_CONTACTS_LIST(Compound);
		OLY_LUT_CONTACTS_LIST(TCompound);
		OLY_LUT_CONTACTS_LIST(BVH<AABB>);
		OLY_LUT_CONTACTS_LIST(TBVH<AABB>);
		OLY_LUT_CONTACTS_LIST(BVH<OBB>);
		OLY_LUT_CONTACTS_LIST(TBVH<OBB>);
		OLY_LUT_CONTACTS_LIST(BVH<KDOP2>);
		OLY_LUT_CONTACTS_LIST(TBVH<KDOP2>);
		OLY_LUT_CONTACTS_LIST(BVH<KDOP3>);
		OLY_LUT_CONTACTS_LIST(TBVH<KDOP3>);
		OLY_LUT_CONTACTS_LIST(BVH<KDOP4>);
		OLY_LUT_CONTACTS_LIST(TBVH<KDOP4>);

#undef OLY_LUT_CONTACTS_LIST
#undef OLY_LUT_CONTACTS
	}

	static void load_circle_cast_hits()
	{
#define OLY_LUT_CIRCLE_CAST_HITS(Class) lut.circle_cast_hits[CObjIDTrait<Class>::ID] = [](const void* ptr, const CircleCast& cast) { return circle_cast_hits(*static_cast<const Class*>(ptr), cast); }

		OLY_LUT_CIRCLE_CAST_HITS(Primitive);
		OLY_LUT_CIRCLE_CAST_HITS(TPrimitive);
		OLY_LUT_CIRCLE_CAST_HITS(Compound);
		OLY_LUT_CIRCLE_CAST_HITS(TCompound);
		OLY_LUT_CIRCLE_CAST_HITS(BVH<AABB>);
		OLY_LUT_CIRCLE_CAST_HITS(TBVH<AABB>);
		OLY_LUT_CIRCLE_CAST_HITS(BVH<OBB>);
		OLY_LUT_CIRCLE_CAST_HITS(TBVH<OBB>);
		OLY_LUT_CIRCLE_CAST_HITS(BVH<KDOP2>);
		OLY_LUT_CIRCLE_CAST_HITS(TBVH<KDOP2>);
		OLY_LUT_CIRCLE_CAST_HITS(BVH<KDOP3>);
		OLY_LUT_CIRCLE_CAST_HITS(TBVH<KDOP3>);
		OLY_LUT_CIRCLE_CAST_HITS(BVH<KDOP4>);
		OLY_LUT_CIRCLE_CAST_HITS(TBVH<KDOP4>);

#undef OLY_LUT_CIRCLE_CAST_HITS
	}

	static void load_rect_cast_hits()
	{
#define OLY_LUT_RECT_CAST_HITS(Class) lut.rect_cast_hits[CObjIDTrait<Class>::ID] = [](const void* ptr, const RectCast& cast) { return rect_cast_hits(*static_cast<const Class*>(ptr), cast); }

		OLY_LUT_RECT_CAST_HITS(Primitive);
		OLY_LUT_RECT_CAST_HITS(TPrimitive);
		OLY_LUT_RECT_CAST_HITS(Compound);
		OLY_LUT_RECT_CAST_HITS(TCompound);
		OLY_LUT_RECT_CAST_HITS(BVH<AABB>);
		OLY_LUT_RECT_CAST_HITS(TBVH<AABB>);
		OLY_LUT_RECT_CAST_HITS(BVH<OBB>);
		OLY_LUT_RECT_CAST_HITS(TBVH<OBB>);
		OLY_LUT_RECT_CAST_HITS(BVH<KDOP2>);
		OLY_LUT_RECT_CAST_HITS(TBVH<KDOP2>);
		OLY_LUT_RECT_CAST_HITS(BVH<KDOP3>);
		OLY_LUT_RECT_CAST_HITS(TBVH<KDOP3>);
		OLY_LUT_RECT_CAST_HITS(BVH<KDOP4>);
		OLY_LUT_RECT_CAST_HITS(TBVH<KDOP4>);

#undef OLY_LUT_CIRCLE_CAST_HITS
	}

	static void load_flush()
	{
		lut.flush[CObjIDTrait<Primitive>::ID] = [](const void* ptr) { return Wrap<AABB>{}(param(static_cast<const Primitive*>(ptr)->element)).rect(); };
		lut.flush[CObjIDTrait<TPrimitive>::ID] = [](const void* ptr) { return Wrap<AABB>{}(param(static_cast<const TPrimitive*>(ptr)->get_baked())).rect(); };
		lut.flush[CObjIDTrait<Compound>::ID] = [](const void* ptr) { return Wrap<AABB>{}(static_cast<const Compound*>(ptr)->elements.data(), static_cast<const Compound*>(ptr)->elements.size()).rect(); };
		lut.flush[CObjIDTrait<TCompound>::ID] = [](const void* ptr) { return Wrap<AABB>{}(static_cast<const TCompound*>(ptr)->get_baked().data(), static_cast<const TCompound*>(ptr)->get_baked().size()).rect(); };

#define OLY_LUT_FLUSH_BVH(Class) lut.flush[CObjIDTrait<Class>::ID] = [](const void* ptr) { return Wrap<AABB>{}(&static_cast<const Class*>(ptr)->root_shape()).rect(); };

		OLY_LUT_FLUSH_BVH(BVH<AABB>);
		OLY_LUT_FLUSH_BVH(TBVH<AABB>);
		OLY_LUT_FLUSH_BVH(BVH<OBB>);
		OLY_LUT_FLUSH_BVH(TBVH<OBB>);
		OLY_LUT_FLUSH_BVH(BVH<KDOP2>);
		OLY_LUT_FLUSH_BVH(TBVH<KDOP2>);
		OLY_LUT_FLUSH_BVH(BVH<KDOP3>);
		OLY_LUT_FLUSH_BVH(TBVH<KDOP3>);
		OLY_LUT_FLUSH_BVH(BVH<KDOP4>);
		OLY_LUT_FLUSH_BVH(TBVH<KDOP4>);

#undef OLY_LUT_FLUSH_BVH
	}

	static void load_is_dirty()
	{
#define OLY_LUT_IS_DIRTY_FALSE(Class) lut.is_dirty[CObjIDTrait<Class>::ID] = [](const void* ptr) { return false; };
#define OLY_LUT_IS_DIRTY_CHECK(Class) lut.is_dirty[CObjIDTrait<Class>::ID] = [](const void* ptr) { return static_cast<const Class*>(ptr)->is_dirty(); };

		OLY_LUT_IS_DIRTY_FALSE(Primitive);
		OLY_LUT_IS_DIRTY_CHECK(TPrimitive);
		OLY_LUT_IS_DIRTY_FALSE(Compound);
		OLY_LUT_IS_DIRTY_CHECK(TCompound);
		OLY_LUT_IS_DIRTY_FALSE(BVH<AABB>);
		OLY_LUT_IS_DIRTY_CHECK(TBVH<AABB>);
		OLY_LUT_IS_DIRTY_FALSE(BVH<OBB>);
		OLY_LUT_IS_DIRTY_CHECK(TBVH<OBB>);
		OLY_LUT_IS_DIRTY_FALSE(BVH<KDOP2>);
		OLY_LUT_IS_DIRTY_CHECK(TBVH<KDOP2>);
		OLY_LUT_IS_DIRTY_FALSE(BVH<KDOP3>);
		OLY_LUT_IS_DIRTY_CHECK(TBVH<KDOP3>);
		OLY_LUT_IS_DIRTY_FALSE(BVH<KDOP4>);
		OLY_LUT_IS_DIRTY_CHECK(TBVH<KDOP4>);

#undef OLY_LUT_IS_DIRTY_FALSE
#undef OLY_LUT_IS_DIRTY_CHECK
	}

	static void load_collision_view()
	{
#define OLY_LUT_COLLISION_VIEW(Class) lut.collision_view[CObjIDTrait<Class>::ID] = [](const void* ptr, glm::vec4 color) { return debug::collision_view(*static_cast<const Class*>(ptr), color); };

		OLY_LUT_COLLISION_VIEW(Primitive);
		OLY_LUT_COLLISION_VIEW(TPrimitive);
		OLY_LUT_COLLISION_VIEW(Compound);
		OLY_LUT_COLLISION_VIEW(TCompound);
		OLY_LUT_COLLISION_VIEW(BVH<AABB>);
		OLY_LUT_COLLISION_VIEW(TBVH<AABB>);
		OLY_LUT_COLLISION_VIEW(BVH<OBB>);
		OLY_LUT_COLLISION_VIEW(TBVH<OBB>);
		OLY_LUT_COLLISION_VIEW(BVH<KDOP2>);
		OLY_LUT_COLLISION_VIEW(TBVH<KDOP2>);
		OLY_LUT_COLLISION_VIEW(BVH<KDOP3>);
		OLY_LUT_COLLISION_VIEW(TBVH<KDOP3>);
		OLY_LUT_COLLISION_VIEW(BVH<KDOP4>);
		OLY_LUT_COLLISION_VIEW(TBVH<KDOP4>);

#undef OLY_LUT_COLLISION_VIEW
	}

	static void load_update_view()
	{
#define OLY_LUT_UPDATE_VIEW(Class) lut.update_view[CObjIDTrait<Class>::ID] = [](debug::CollisionView& view, const void* ptr, glm::vec4 color) { debug::update_view(view, *static_cast<const Class*>(ptr), color); };

		OLY_LUT_UPDATE_VIEW(Primitive);
		OLY_LUT_UPDATE_VIEW(TPrimitive);
		OLY_LUT_UPDATE_VIEW(Compound);
		OLY_LUT_UPDATE_VIEW(TCompound);
		OLY_LUT_UPDATE_VIEW(BVH<AABB>);
		OLY_LUT_UPDATE_VIEW(TBVH<AABB>);
		OLY_LUT_UPDATE_VIEW(BVH<OBB>);
		OLY_LUT_UPDATE_VIEW(TBVH<OBB>);
		OLY_LUT_UPDATE_VIEW(BVH<KDOP2>);
		OLY_LUT_UPDATE_VIEW(TBVH<KDOP2>);
		OLY_LUT_UPDATE_VIEW(BVH<KDOP3>);
		OLY_LUT_UPDATE_VIEW(TBVH<KDOP3>);
		OLY_LUT_UPDATE_VIEW(BVH<KDOP4>);
		OLY_LUT_UPDATE_VIEW(TBVH<KDOP4>);

#undef OLY_LUT_UPDATE_VIEW
	}

	void load_luts()
	{
		load_point_hits();
		load_ray_hits();
		load_raycast();
		load_overlaps();
		load_collides();
		load_contacts();
		load_circle_cast_hits();
		load_rect_cast_hits();

		load_flush();
		load_is_dirty();
		load_collision_view();
		load_update_view();
	}

	OverlapResult lut_point_hits(const ColliderObject& c, glm::vec2 test)
	{
		return (lut.point_hits[c.id()])(c.raw_obj(), test);
	}

	OverlapResult lut_ray_hits(const ColliderObject& c, const Ray& ray)
	{
		return (lut.ray_hits[c.id()])(c.raw_obj(), ray);
	}

	RaycastResult lut_raycast(const ColliderObject& c, const Ray& ray)
	{
		return (lut.raycast[c.id()])(c.raw_obj(), ray);
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

	OverlapResult lut_circle_cast_hits(const ColliderObject& c, const CircleCast& cast)
	{
		return (lut.circle_cast_hits[c.id()])(c.raw_obj(), cast);
	}

	OverlapResult lut_rect_cast_hits(const ColliderObject& c, const RectCast& cast)
	{
		return (lut.rect_cast_hits[c.id()])(c.raw_obj(), cast);
	}

	math::Rect2D lut_flush(const ColliderObject& c)
	{
		return (lut.flush[c.id()])(c.raw_obj());
	}
	
	bool lut_is_dirty(const ColliderObject& c)
	{
		return (lut.is_dirty[c.id()])(c.raw_obj());
	}

	debug::CollisionView lut_collision_view(const ColliderObject& c, glm::vec4 color)
	{
		return (lut.collision_view[c.id()])(c.raw_obj(), color);
	}
	
	void lut_update_view(debug::CollisionView& view, const ColliderObject& c, glm::vec4 color)
	{
		(lut.update_view[c.id()])(view, c.raw_obj(), color);
	}
}
