#pragma once

#include "physics/collision/objects/Polygon.h"

#include "physics/collision/scene/dispatch/CollisionDispatcher.h"

#include "physics/dynamics/components/DynamicsComponent.h"

namespace oly::physics
{
	namespace internal
	{
		struct RigidBodyOnTick;
	}

	class RigidBody
        : public col2d::CollisionController
        , public imp::auto_trackable<RigidBody>
	{
        friend class internal::RigidBodyOnTick;

	protected:
		std::vector<col2d::Collider> colliders_;
		Transformer2DRef transformer_ = REF_INIT;

	public:
        bool collision_enabled = true;

		RigidBody();
		RigidBody(const Transformer2DRef& ref);
		RigidBody(const RigidBody&);
		RigidBody(RigidBody&&) noexcept;
		virtual ~RigidBody() = default;

		RigidBody& operator=(const RigidBody&);
		RigidBody& operator=(RigidBody&&) noexcept;

    public:
        virtual State state() const = 0;
        virtual bool is_colliding() const = 0;

    protected:
        virtual void physics_pre_tick() = 0;
        virtual void physics_post_tick() = 0;

        virtual void bind(const col2d::Collider& collider) const = 0;
        virtual void unbind(const col2d::Collider& collider) const = 0;

        virtual const DynamicsComponent& get_dynamics() const = 0;

    public:
        Transformer2DExposure<> get_transformer() const;
		Transformer2DExposure<ex::Full> set_transformer();
        void assign_transformer(const Transformer2DRef& ref);

        const Transform2D& get_local() const;
        Transform2D& set_local();

		col2d::Collider& add_collider(col2d::Collider&& collider);
		
        template<col2d::internal::CObj_check CObj>
		col2d::Collider& add_collider(CObj&& obj)
        {
            return add_collider(col2d::Collider(std::forward<CObj>(obj)));
        }
		
        template<col2d::internal::Elem_check Shape>
		col2d::Collider& add_collider(Shape&& obj)
        {
            return add_collider(col2d::TPrimitive(std::forward<Shape>(obj)));
        }

        col2d::Collider& add_collider(const col2d::Capsule& capsule);
        col2d::Collider& add_collider(const col2d::PolygonCollision& polygon);

		void erase_collider(size_t i);
        void clear_colliders();

        const col2d::Collider& collider(size_t i = 0) const;
        col2d::Collider& collider(size_t i = 0);
        size_t collider_count() const;
        size_t collider_index(const col2d::Collider& col) const;

        debug::DebugOverlay create_debug_overlay(
            debug::DebugOverlayLayer& layer,
            size_t i,
            glm::vec4 color,
            debug::DebugOverlay::PaintOptions paint_options = {}) const;

        void modify_debug_overlay(size_t i, debug::DebugOverlay& overlay) const;

    protected:
        void bind_all() const;
        void unbind_all() const;

        static const RigidBody* rigid_body(const col2d::Collider& collider);
        static const DynamicsComponent& dynamics_of(const RigidBody& other);

	private:
		void remove_collider(const col2d::Collider& collider);
	};

    using RigidBodyRef = SmartReference<RigidBody>;
}
