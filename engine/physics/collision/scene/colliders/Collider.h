#pragma once

#include "core/base/TransformerExposure.h"

#include "physics/collision/scene/colliders/ColliderDispatchHandle.h"
#include "physics/collision/scene/colliders/ColliderObject.h"
#include "physics/collision/scene/colliders/TreeHandleMap.h"

#include "physics/collision/scene/luts/LUT.h"
#include "physics/collision/scene/luts/LUTVariant.h"

namespace oly::physics
{
    class RigidBody;
};

namespace oly::col2d
{
	class Collider
	{
		friend class CollisionTree;
		friend class internal::CollisionNode;
		friend class physics::RigidBody;

	private:
		internal::ColliderObject obj_;
		physics::RigidBody* rigid_body_ = nullptr;
		internal::ColliderDispatchHandle dispatch_handle_;

	protected:
		mutable math::Rect2D quad_wrap_;

	public:
		internal::TreeHandleMap handles = internal::TreeHandleMap(*this);
        std::optional<UnitVector2D> one_way_blocking;

    private:
        mutable bool dirty_ = true;

    public:
        Collider();

        template<internal::CObj_check CObj>
		explicit Collider(CObj&& obj)
            : obj_(std::forward<CObj>(obj))
            , dispatch_handle_(*this)
        {
        }
		
        Collider(internal::ColliderObject&& obj);
		Collider(const Collider&);
		Collider(Collider&&) noexcept;
		
        Collider& operator=(const Collider&);
		Collider& operator=(Collider&&) noexcept;

    public:
		template<internal::CObj_check CObj>
		const CObj& get() const
        {
            return obj_.get<CObj>();
        }
		
        template<internal::CObj_check CObj>
		CObj& set()
        {
            dirty_ = true;
            return obj_.set<CObj>();
        }
		
        void emplace(internal::ColliderObject&& obj);
        
        template<internal::CObj_check CObj>
        void emplace(CObj&& obj)
        {
            emplace(internal::ColliderObject(std::forward<CObj>(obj)));
        }

        void flag();

        const Transform2D& get_local() const;
        Transform2D& set_local();

        Transformer2DExposure<> get_transformer() const;
        Transformer2DExposure<ex::kFull> set_transformer();

        Layer layer() const;
        Layer& layer();
        Mask mask() const;
        Mask& mask();

		bool one_way_blocks(const Collider& active) const;

        OverlapResult point_hits(glm::vec2 test) const;
        OverlapResult ray_hits(Ray ray) const;
        RaycastResult raycast(Ray ray) const;
		OverlapResult overlaps(const Collider& other) const;
		CollisionResult collides(const Collider& other) const;
		ContactResult contacts(const Collider& other) const;
		OverlapResult circle_cast_hits(const CircleCast& cast) const;
		OverlapResult rect_cast_hits(const RectCast& cast) const;

        debug::DebugOverlay create_debug_overlay(
            debug::DebugOverlayLayer& layer,
            glm::vec4 color,
            debug::DebugOverlay::PaintOptions paint_options = {}) const;

        void modify_debug_overlay(debug::DebugOverlay& overlay, size_t shape_index = 0) const;

	private:
        bool is_dirty() const;
        void flush() const;

        internal::ColliderObjectConstVariant get_object_variant() const;
        internal::ColliderObjectVariant get_object_variant();
	};
}
