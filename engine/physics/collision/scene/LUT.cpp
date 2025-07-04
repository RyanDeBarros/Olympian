#include "LUT.h"

#include "physics/collision/scene/LUTVariant.h"

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

	using TransformerFn = const Transformer2D&(*)(const void*);
		
	struct LUT
	{
		PointHitsFn point_hits_[CObjID::_COUNT];
		RayHitsFn ray_hits_[CObjID::_COUNT];
		RaycastFn raycast_[CObjID::_COUNT];
		OverlapsFn overlaps_[CObjID::_COUNT][CObjID::_COUNT];
		CollidesFn collides_[CObjID::_COUNT][CObjID::_COUNT];
		ContactsFn contacts_[CObjID::_COUNT][CObjID::_COUNT];
		CircleCastHitsFn circle_cast_hits_[CObjID::_COUNT];
		RectCastHitsFn rect_cast_hits_[CObjID::_COUNT];

		FlushFn flush_[CObjID::_COUNT];
		IsDirtyFn is_dirty_[CObjID::_COUNT];
		CollisionViewFn collision_view_[CObjID::_COUNT];
		UpdateViewFn update_view_[CObjID::_COUNT];

		TransformerFn transformer_[CObjID::_COUNT];

		void load_point_hits()
		{
#define OLY_LUT_POINT_HITS(Class) point_hits_[CObjIDTrait<Class>::ID] = [](const void* ptr, glm::vec2 test) { return point_hits(*static_cast<const Class*>(ptr), test); };

			OLY_LUT_POINT_HITS(TPrimitive);
			OLY_LUT_POINT_HITS(TCompound);
			OLY_LUT_POINT_HITS(TBVH<AABB>);
			OLY_LUT_POINT_HITS(TBVH<OBB>);
			OLY_LUT_POINT_HITS(TBVH<KDOP2>);
			OLY_LUT_POINT_HITS(TBVH<KDOP3>);
			OLY_LUT_POINT_HITS(TBVH<KDOP4>);

#undef OLY_LUT_POINT_HITS
		}

		void load_ray_hits()
		{
#define OLY_LUT_RAY_HITS(Class) ray_hits_[CObjIDTrait<Class>::ID] = [](const void* ptr, const Ray& ray) { return ray_hits(*static_cast<const Class*>(ptr), ray); };

			OLY_LUT_RAY_HITS(TPrimitive);
			OLY_LUT_RAY_HITS(TCompound);
			OLY_LUT_RAY_HITS(TBVH<AABB>);
			OLY_LUT_RAY_HITS(TBVH<OBB>);
			OLY_LUT_RAY_HITS(TBVH<KDOP2>);
			OLY_LUT_RAY_HITS(TBVH<KDOP3>);
			OLY_LUT_RAY_HITS(TBVH<KDOP4>);

#undef OLY_LUT_RAY_HITS
		}

		void load_raycast()
		{
#define OLY_LUT_RAYCAST(Class) raycast_[CObjIDTrait<Class>::ID] = [](const void* ptr, const Ray& ray) { return raycast(*static_cast<const Class*>(ptr), ray); };

			OLY_LUT_RAYCAST(TPrimitive);
			OLY_LUT_RAYCAST(TCompound);
			OLY_LUT_RAYCAST(TBVH<AABB>);
			OLY_LUT_RAYCAST(TBVH<OBB>);
			OLY_LUT_RAYCAST(TBVH<KDOP2>);
			OLY_LUT_RAYCAST(TBVH<KDOP3>);
			OLY_LUT_RAYCAST(TBVH<KDOP4>);

#undef OLY_LUT_RAYCAST
		}

		void load_overlaps()
		{
#define OLY_LUT_OVERLAPS(Class1, Class2) overlaps_[CObjIDTrait<Class1>::ID][CObjIDTrait<Class2>::ID] = [](const void* ptr1, const void* ptr2)\
				{ return overlaps(*static_cast<const Class1*>(ptr1), *static_cast<const Class2*>(ptr2)); };

#define OLY_LUT_OVERLAPS_LIST(Class)\
			OLY_LUT_OVERLAPS(Class, TPrimitive);\
			OLY_LUT_OVERLAPS(Class, TCompound);\
			OLY_LUT_OVERLAPS(Class, TBVH<AABB>);\
			OLY_LUT_OVERLAPS(Class, TBVH<OBB>);\
			OLY_LUT_OVERLAPS(Class, TBVH<KDOP2>);\
			OLY_LUT_OVERLAPS(Class, TBVH<KDOP3>);\
			OLY_LUT_OVERLAPS(Class, TBVH<KDOP4>);

			OLY_LUT_OVERLAPS_LIST(TPrimitive);
			OLY_LUT_OVERLAPS_LIST(TCompound);
			OLY_LUT_OVERLAPS_LIST(TBVH<AABB>);
			OLY_LUT_OVERLAPS_LIST(TBVH<OBB>);
			OLY_LUT_OVERLAPS_LIST(TBVH<KDOP2>);
			OLY_LUT_OVERLAPS_LIST(TBVH<KDOP3>);
			OLY_LUT_OVERLAPS_LIST(TBVH<KDOP4>);

#undef OLY_LUT_OVERLAPS_LIST
#undef OLY_LUT_OVERLAPS
		}

		void load_collides()
		{
#define OLY_LUT_COLLIDES(Class1, Class2) collides_[CObjIDTrait<Class1>::ID][CObjIDTrait<Class2>::ID] = [](const void* ptr1, const void* ptr2)\
				{ return collides(*static_cast<const Class1*>(ptr1), *static_cast<const Class2*>(ptr2)); };

#define OLY_LUT_COLLIDES_LIST(Class)\
			OLY_LUT_COLLIDES(Class, TPrimitive);\
			OLY_LUT_COLLIDES(Class, TCompound);\
			OLY_LUT_COLLIDES(Class, TBVH<AABB>);\
			OLY_LUT_COLLIDES(Class, TBVH<OBB>);\
			OLY_LUT_COLLIDES(Class, TBVH<KDOP2>);\
			OLY_LUT_COLLIDES(Class, TBVH<KDOP3>);\
			OLY_LUT_COLLIDES(Class, TBVH<KDOP4>);

			OLY_LUT_COLLIDES_LIST(TPrimitive);
			OLY_LUT_COLLIDES_LIST(TCompound);
			OLY_LUT_COLLIDES_LIST(TBVH<AABB>);
			OLY_LUT_COLLIDES_LIST(TBVH<OBB>);
			OLY_LUT_COLLIDES_LIST(TBVH<KDOP2>);
			OLY_LUT_COLLIDES_LIST(TBVH<KDOP3>);
			OLY_LUT_COLLIDES_LIST(TBVH<KDOP4>);

#undef OLY_LUT_COLLIDES_LIST
#undef OLY_LUT_COLLIDES
		}

		void load_contacts()
		{
#define OLY_LUT_CONTACTS(Class1, Class2) contacts_[CObjIDTrait<Class1>::ID][CObjIDTrait<Class2>::ID] = [](const void* ptr1, const void* ptr2)\
				{ return contacts(*static_cast<const Class1*>(ptr1), *static_cast<const Class2*>(ptr2)); };

#define OLY_LUT_CONTACTS_LIST(Class)\
			OLY_LUT_CONTACTS(Class, TPrimitive);\
			OLY_LUT_CONTACTS(Class, TCompound);\
			OLY_LUT_CONTACTS(Class, TBVH<AABB>);\
			OLY_LUT_CONTACTS(Class, TBVH<OBB>);\
			OLY_LUT_CONTACTS(Class, TBVH<KDOP2>);\
			OLY_LUT_CONTACTS(Class, TBVH<KDOP3>);\
			OLY_LUT_CONTACTS(Class, TBVH<KDOP4>);

			OLY_LUT_CONTACTS_LIST(TPrimitive);
			OLY_LUT_CONTACTS_LIST(TCompound);
			OLY_LUT_CONTACTS_LIST(TBVH<AABB>);
			OLY_LUT_CONTACTS_LIST(TBVH<OBB>);
			OLY_LUT_CONTACTS_LIST(TBVH<KDOP2>);
			OLY_LUT_CONTACTS_LIST(TBVH<KDOP3>);
			OLY_LUT_CONTACTS_LIST(TBVH<KDOP4>);

#undef OLY_LUT_CONTACTS_LIST
#undef OLY_LUT_CONTACTS
		}

		void load_circle_cast_hits()
		{
#define OLY_LUT_CIRCLE_CAST_HITS(Class) circle_cast_hits_[CObjIDTrait<Class>::ID] = [](const void* ptr, const CircleCast& cast) { return circle_cast_hits(*static_cast<const Class*>(ptr), cast); }

			OLY_LUT_CIRCLE_CAST_HITS(TPrimitive);
			OLY_LUT_CIRCLE_CAST_HITS(TCompound);
			OLY_LUT_CIRCLE_CAST_HITS(TBVH<AABB>);
			OLY_LUT_CIRCLE_CAST_HITS(TBVH<OBB>);
			OLY_LUT_CIRCLE_CAST_HITS(TBVH<KDOP2>);
			OLY_LUT_CIRCLE_CAST_HITS(TBVH<KDOP3>);
			OLY_LUT_CIRCLE_CAST_HITS(TBVH<KDOP4>);

#undef OLY_LUT_CIRCLE_CAST_HITS
		}

		void load_rect_cast_hits()
		{
#define OLY_LUT_RECT_CAST_HITS(Class) rect_cast_hits_[CObjIDTrait<Class>::ID] = [](const void* ptr, const RectCast& cast) { return rect_cast_hits(*static_cast<const Class*>(ptr), cast); }

			OLY_LUT_RECT_CAST_HITS(TPrimitive);
			OLY_LUT_RECT_CAST_HITS(TCompound);
			OLY_LUT_RECT_CAST_HITS(TBVH<AABB>);
			OLY_LUT_RECT_CAST_HITS(TBVH<OBB>);
			OLY_LUT_RECT_CAST_HITS(TBVH<KDOP2>);
			OLY_LUT_RECT_CAST_HITS(TBVH<KDOP3>);
			OLY_LUT_RECT_CAST_HITS(TBVH<KDOP4>);

#undef OLY_LUT_CIRCLE_CAST_HITS
		}

		void load_flush()
		{
			flush_[CObjIDTrait<TPrimitive>::ID] = [](const void* ptr) { return Wrap<AABB>{}(param(static_cast<const TPrimitive*>(ptr)->get_baked())).rect(); };
			flush_[CObjIDTrait<TCompound>::ID] = [](const void* ptr) { return Wrap<AABB>{}(static_cast<const TCompound*>(ptr)->get_baked().data(), static_cast<const TCompound*>(ptr)->get_baked().size()).rect(); };

#define OLY_LUT_FLUSH_BVH(Class) flush_[CObjIDTrait<Class>::ID] = [](const void* ptr) { return Wrap<AABB>{}(&static_cast<const Class*>(ptr)->root_shape()).rect(); };

			OLY_LUT_FLUSH_BVH(TBVH<AABB>);
			OLY_LUT_FLUSH_BVH(TBVH<OBB>);
			OLY_LUT_FLUSH_BVH(TBVH<KDOP2>);
			OLY_LUT_FLUSH_BVH(TBVH<KDOP3>);
			OLY_LUT_FLUSH_BVH(TBVH<KDOP4>);

#undef OLY_LUT_FLUSH_BVH
		}

		void load_is_dirty()
		{
#define OLY_LUT_IS_DIRTY(Class) is_dirty_[CObjIDTrait<Class>::ID] = [](const void* ptr) { return static_cast<const Class*>(ptr)->is_dirty(); };

			OLY_LUT_IS_DIRTY(TPrimitive);
			OLY_LUT_IS_DIRTY(TCompound);
			OLY_LUT_IS_DIRTY(TBVH<AABB>);
			OLY_LUT_IS_DIRTY(TBVH<OBB>);
			OLY_LUT_IS_DIRTY(TBVH<KDOP2>);
			OLY_LUT_IS_DIRTY(TBVH<KDOP3>);
			OLY_LUT_IS_DIRTY(TBVH<KDOP4>);

#undef OLY_LUT_IS_DIRTY
		}

		void load_collision_view()
		{
#define OLY_LUT_COLLISION_VIEW(Class) collision_view_[CObjIDTrait<Class>::ID] = [](const void* ptr, glm::vec4 color) { return debug::collision_view(*static_cast<const Class*>(ptr), color); };

			OLY_LUT_COLLISION_VIEW(TPrimitive);
			OLY_LUT_COLLISION_VIEW(TCompound);
			OLY_LUT_COLLISION_VIEW(TBVH<AABB>);
			OLY_LUT_COLLISION_VIEW(TBVH<OBB>);
			OLY_LUT_COLLISION_VIEW(TBVH<KDOP2>);
			OLY_LUT_COLLISION_VIEW(TBVH<KDOP3>);
			OLY_LUT_COLLISION_VIEW(TBVH<KDOP4>);

#undef OLY_LUT_COLLISION_VIEW
		}

		void load_update_view()
		{
#define OLY_LUT_UPDATE_VIEW(Class) update_view_[CObjIDTrait<Class>::ID] = [](debug::CollisionView& view, const void* ptr, glm::vec4 color) { debug::update_view(view, *static_cast<const Class*>(ptr), color); };

			OLY_LUT_UPDATE_VIEW(TPrimitive);
			OLY_LUT_UPDATE_VIEW(TCompound);
			OLY_LUT_UPDATE_VIEW(TBVH<AABB>);
			OLY_LUT_UPDATE_VIEW(TBVH<OBB>);
			OLY_LUT_UPDATE_VIEW(TBVH<KDOP2>);
			OLY_LUT_UPDATE_VIEW(TBVH<KDOP3>);
			OLY_LUT_UPDATE_VIEW(TBVH<KDOP4>);

#undef OLY_LUT_UPDATE_VIEW
		}

		void load_transformer()
		{
#define OLY_LUT_TRANSFORMER(Class) transformer_[CObjIDTrait<Class>::ID] = [](const void* ptr) -> const Transformer2D& { return static_cast<const Class*>(ptr)->transformer; };

			OLY_LUT_TRANSFORMER(TPrimitive);
			OLY_LUT_TRANSFORMER(TCompound);
			OLY_LUT_TRANSFORMER(TBVH<AABB>);
			OLY_LUT_TRANSFORMER(TBVH<OBB>);
			OLY_LUT_TRANSFORMER(TBVH<KDOP2>);
			OLY_LUT_TRANSFORMER(TBVH<KDOP3>);
			OLY_LUT_TRANSFORMER(TBVH<KDOP4>);

#undef OLY_LUT_TRANSFORMER
		}
	} static lut;

	void load_luts()
	{
		lut.load_point_hits();
		lut.load_ray_hits();
		lut.load_raycast();
		lut.load_overlaps();
		lut.load_collides();
		lut.load_contacts();
		lut.load_circle_cast_hits();
		lut.load_rect_cast_hits();

		lut.load_flush();
		lut.load_is_dirty();
		lut.load_collision_view();
		lut.load_update_view();

		lut.load_transformer();

		load_variant_luts();
	}

	OverlapResult lut_point_hits(const ColliderObject& c, glm::vec2 test)
	{
		return (lut.point_hits_[c.id()])(c.raw_obj(), test);
	}

	OverlapResult lut_ray_hits(const ColliderObject& c, const Ray& ray)
	{
		return (lut.ray_hits_[c.id()])(c.raw_obj(), ray);
	}

	RaycastResult lut_raycast(const ColliderObject& c, const Ray& ray)
	{
		return (lut.raycast_[c.id()])(c.raw_obj(), ray);
	}

	OverlapResult lut_overlaps(const ColliderObject& c1, const ColliderObject& c2)
	{
		return (lut.overlaps_[c1.id()][c2.id()])(c1.raw_obj(), c2.raw_obj());
	}
	
	CollisionResult lut_collides(const ColliderObject& c1, const ColliderObject& c2)
	{
		return (lut.collides_[c1.id()][c2.id()])(c1.raw_obj(), c2.raw_obj());
	}
	
	ContactResult lut_contacts(const ColliderObject& c1, const ColliderObject& c2)
	{
		return (lut.contacts_[c1.id()][c2.id()])(c1.raw_obj(), c2.raw_obj());
	}

	OverlapResult lut_circle_cast_hits(const ColliderObject& c, const CircleCast& cast)
	{
		return (lut.circle_cast_hits_[c.id()])(c.raw_obj(), cast);
	}

	OverlapResult lut_rect_cast_hits(const ColliderObject& c, const RectCast& cast)
	{
		return (lut.rect_cast_hits_[c.id()])(c.raw_obj(), cast);
	}

	math::Rect2D lut_flush(const ColliderObject& c)
	{
		return (lut.flush_[c.id()])(c.raw_obj());
	}
	
	bool lut_is_dirty(const ColliderObject& c)
	{
		return (lut.is_dirty_[c.id()])(c.raw_obj());
	}

	debug::CollisionView lut_collision_view(const ColliderObject& c, glm::vec4 color)
	{
		return (lut.collision_view_[c.id()])(c.raw_obj(), color);
	}
	
	void lut_update_view(debug::CollisionView& view, const ColliderObject& c, glm::vec4 color)
	{
		(lut.update_view_[c.id()])(view, c.raw_obj(), color);
	}

	const Transformer2D& lut_transformer(const ColliderObject& c)
	{
		return (lut.transformer_[c.id()])(c.raw_obj());
	}

	Transformer2D& lut_transformer(ColliderObject& c)
	{
		return *const_cast<Transformer2D*>(&(lut.transformer_[c.id()])(const_cast<void*>(c.raw_obj())));
	}
}
