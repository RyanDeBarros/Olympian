#pragma once

#include "physics/collision/scene/CollisionTree.h"
#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"

namespace oly::col2d
{
	class PrimitiveCollider : public ICollider
	{
		Primitive p;
		mutable bool _d = true;
		mutable math::Rect2D wrap;

	public:
		math::Rect2D quad_wrap() const override;

		const Primitive& get() const { return p; }
		Primitive& set() { _d = true; return p; }
	};

	class TPrimitiveCollider : public ICollider
	{
		TPrimitive p;
		mutable bool _d = true;
		mutable math::Rect2D wrap;

	public:
		math::Rect2D quad_wrap() const override;

		const TPrimitive& get() const { return p; }
		TPrimitive& set() { _d = true; return p; }
	};

	class CompoundCollider : public ICollider
	{
		Compound c;
		mutable bool _d = true;
		mutable math::Rect2D wrap;

	public:
		math::Rect2D quad_wrap() const override;

		const Compound& get() const { return c; }
		Compound& set() { _d = true; return c; }
	};

	class TCompoundCollider : public ICollider
	{
		TCompound c;
		mutable bool _d = true;
		mutable math::Rect2D wrap;

	public:
		math::Rect2D quad_wrap() const override;

		const TCompound& get() const { return c; }
		TCompound& set() { _d = true; return c; }
	};

	template<typename Shape>
	class BVHCollider : public ICollider
	{
		BVH<Shape> b;
		mutable bool _d = true;
		mutable math::Rect2D wrap;

	public:
		math::Rect2D quad_wrap() const override
		{
			if (_d)
			{
				_d = false;
				wrap = internal::Wrap<AABB>{}(&b.root_shape()).rect();
			}
			return wrap;
		}

		const BVH<Shape>& get() const { return b; }
		BVH<Shape>& set() { _d = true; return b; }
	};

	template<typename Shape>
	class TBVHCollider : public ICollider
	{
		TBVH<Shape> b;
		mutable bool _d = true;
		mutable math::Rect2D wrap;

	public:
		math::Rect2D quad_wrap() const override
		{
			if (_d)
			{
				_d = false;
				wrap = internal::Wrap<AABB>{}(&b.root_shape()).rect();
			}
			return wrap;
		}

		const TBVH<Shape>& get() const { return b; }
		TBVH<Shape>& set() { _d = true; return b; }
	};
}
