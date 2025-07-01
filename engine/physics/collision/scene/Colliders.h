#pragma once

#include "physics/collision/scene/CollisionTree.h"
#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"

namespace oly::col2d
{
	class PrimitiveCollider : public Collider
	{
		Primitive p;

	public:
		using Collider::Collider;

		const Primitive& get() const { return p; }
		Primitive& set() { flag(); return p; }

		ConstSoftReference<PrimitiveCollider> ref() const { return _ref.cref(this); }
		ConstSoftReference<PrimitiveCollider> cref() const { return _ref.cref(this); }
		SoftReference<PrimitiveCollider> ref() { return _ref.ref(this); }

	protected:
		void flush_impl() const override { quad_wrap = internal::Wrap<AABB>{}(param(p.element)).rect(); }
	};

	class TPrimitiveCollider : public Collider
	{
		TPrimitive p;

	public:
		using Collider::Collider;

		const TPrimitive& get() const { return p; }
		TPrimitive& set() { flag(); return p; }

		ConstSoftReference<TPrimitiveCollider> ref() const { return _ref.cref(this); }
		ConstSoftReference<TPrimitiveCollider> cref() const { return _ref.cref(this); }
		SoftReference<TPrimitiveCollider> ref() { return _ref.ref(this); }

	protected:
		bool dirty_impl() const override { return p.is_dirty(); }
		void flush_impl() const override { quad_wrap = internal::Wrap<AABB>{}(param(p.get_baked())).rect(); }
	};

	class CompoundCollider : public Collider
	{
		Compound c;

	public:
		using Collider::Collider;

		const Compound& get() const { return c; }
		Compound& set() { flag(); return c; }

		ConstSoftReference<CompoundCollider> ref() const { return _ref.cref(this); }
		ConstSoftReference<CompoundCollider> cref() const { return _ref.cref(this); }
		SoftReference<CompoundCollider> ref() { return _ref.ref(this); }

	protected:
		void flush_impl() const override { quad_wrap = internal::Wrap<AABB>{}(c.elements.data(), c.elements.size()).rect(); }
	};

	class TCompoundCollider : public Collider
	{
		TCompound c;

	public:
		using Collider::Collider;

		const TCompound& get() const { return c; }
		TCompound& set() { flag(); return c; }

		ConstSoftReference<TCompoundCollider> ref() const { return _ref.cref(this); }
		ConstSoftReference<TCompoundCollider> cref() const { return _ref.cref(this); }
		SoftReference<TCompoundCollider> ref() { return _ref.ref(this); }

	protected:
		bool dirty_impl() const override { return c.is_dirty(); }
		void flush_impl() const override { quad_wrap = internal::Wrap<AABB>{}(c.get_baked().data(), c.get_baked().size()).rect(); }
	};

	template<typename Shape>
	class BVHCollider : public Collider
	{
		BVH<Shape> b;

	public:
		using Collider::Collider;

		const BVH<Shape>& get() const { return b; }
		BVH<Shape>& set() { flag(); return b; }

		ConstSoftReference<BVHCollider<Shape>> ref() const { return _ref.cref(this); }
		ConstSoftReference<BVHCollider<Shape>> cref() const { return _ref.cref(this); }
		SoftReference<BVHCollider<Shape>> ref() { return _ref.ref(this); }

	protected:
		void flush_impl() const override { quad_wrap = internal::Wrap<AABB>{}(&b.root_shape()).rect(); }
	};

	template<typename Shape>
	class TBVHCollider : public Collider
	{
		TBVH<Shape> b;

	public:
		using Collider::Collider;

		const TBVH<Shape>& get() const { return b; }
		TBVH<Shape>& set() { flag(); return b; }

		ConstSoftReference<TBVHCollider<Shape>> ref() const { return _ref.cref(this); }
		ConstSoftReference<TBVHCollider<Shape>> cref() const { return _ref.cref(this); }
		SoftReference<TBVHCollider<Shape>> ref() { return _ref.ref(this); }

	protected:
		bool dirty_impl() const override { return b.is_dirty(); }
		void flush_impl() const override { quad_wrap = internal::Wrap<AABB>{}(&b.root_shape()).rect(); }
	};
}
