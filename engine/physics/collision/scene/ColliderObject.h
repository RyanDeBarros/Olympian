#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/objects/Combinations.h"

#include "core/math/Shapes.h"
#include "core/containers/BlackBox.h"

namespace oly::col2d
{
	// TODO use fn-ptr lookup table over variant
	
	namespace internal
	{
		enum CObjID : unsigned int;

		template<typename T>
		struct CObjIDTrait;

		class ColliderObject
		{
			BlackBox<true> _obj;
			CObjID _id;

		public:
			template<typename CObj>
			explicit ColliderObject(CObj&& shape)
				: _obj(std::forward<CObj>(shape)), _id(CObjIDTrait<std::decay_t<Shape>>::ID)
			{
			}

			const void* raw_obj() const { return _obj.raw(); }
			CObjID id() const { return _id; }
		};
	}


	using VColliderObject = std::variant<
		Primitive,
		TPrimitive,
		Compound,
		TCompound,
		BVH<AABB>,
		TBVH<AABB>,
		BVH<OBB>,
		TBVH<OBB>,
		BVH<KDOP2>,
		TBVH<KDOP2>,
		BVH<KDOP3>,
		TBVH<KDOP3>,
		BVH<KDOP4>,
		TBVH<KDOP4>
	>;

	inline OverlapResult overlaps(const VColliderObject& c1, const VColliderObject& c2)
	{
		return std::visit([&c2](const auto& c1) { return std::visit([&c1](const auto& c2) { return overlaps(c1, c2); }, c2); }, c1);
	}

	inline CollisionResult collides(const VColliderObject& c1, const VColliderObject& c2)
	{
		return std::visit([&c2](const auto& c1) { return std::visit([&c1](const auto& c2) { return collides(c1, c2); }, c2); }, c1);
	}

	inline ContactResult contacts(const VColliderObject& c1, const VColliderObject& c2)
	{
		return std::visit([&c2](const auto& c1) { return std::visit([&c1](const auto& c2) { return contacts(c1, c2); }, c2); }, c1);
	}

	namespace internal
	{
		inline bool shape_is_dirty_impl(const Primitive& p)
		{
			return false;
		}

		inline bool shape_is_dirty_impl(const TPrimitive& p)
		{
			return p.is_dirty();
		}

		inline bool shape_is_dirty_impl(const Compound& c)
		{
			return false;
		}

		inline bool shape_is_dirty_impl(const TCompound& c)
		{
			return c.is_dirty();
		}

		template<typename Shape>
		inline bool shape_is_dirty_impl(const BVH<Shape>& b)
		{
			return false;
		}

		template<typename Shape>
		inline bool shape_is_dirty_impl(const TBVH<Shape>& b)
		{
			return b.is_dirty();
		}

		inline bool shape_is_dirty(const VColliderObject& shape)
		{
			return std::visit([](const auto& shape) -> bool { return shape_is_dirty_impl(shape); }, shape);
		}

		inline math::Rect2D flush_shape_impl(const Primitive& p)
		{
			return internal::Wrap<AABB>{}(param(p.element)).rect();
		}

		inline math::Rect2D flush_shape_impl(const TPrimitive& p)
		{
			return internal::Wrap<AABB>{}(param(p.get_baked())).rect();
		}

		inline math::Rect2D flush_shape_impl(const Compound& c)
		{
			return internal::Wrap<AABB>{}(c.elements.data(), c.elements.size()).rect();
		}

		inline math::Rect2D flush_shape_impl(const TCompound& c)
		{
			return internal::Wrap<AABB>{}(c.get_baked().data(), c.get_baked().size()).rect();
		}

		template<typename Shape>
		inline math::Rect2D flush_shape_impl(const BVH<Shape>& b)
		{
			return internal::Wrap<AABB>{}(&b.root_shape()).rect();
		}

		template<typename Shape>
		inline math::Rect2D flush_shape_impl(const TBVH<Shape>& b)
		{
			return internal::Wrap<AABB>{}(&b.root_shape()).rect();
		}

		inline math::Rect2D flush_shape(const VColliderObject& shape)
		{
			return std::visit([](const auto& shape) -> math::Rect2D { return flush_shape_impl(shape); }, shape);
		}
	}
}
