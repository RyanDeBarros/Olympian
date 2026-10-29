#pragma once

#include "physics/dynamics/bodies/RigidBody.h"

#include "physics/dynamics/components/LinearPhysicsComponent.h"

#include <imp/action.hpp>

namespace oly::physics
{
    class LinearBody
        : public RigidBody
    {
    private:
        LinearPhysicsComponent dynamics_;

    public:
        imp::action_delegator<col2d::CollisionEventData> delegator;

        LinearBody();
        LinearBody(const LinearBody&) = delete;
        LinearBody(LinearBody&&) noexcept;
        ~LinearBody();

        LinearBody& operator=(const LinearBody&) = delete;
        LinearBody& operator=(LinearBody&&) noexcept;

    public:
        State state() const;
        bool is_colliding() const;

    protected:
        void physics_pre_tick() override;
        void physics_post_tick() override;

        void bind(const col2d::Collider& collider) const override;
        void unbind(const col2d::Collider& collider) const override;

        const DynamicsComponent& get_dynamics() const override;

    public:
        const LinearPhysicsComponent& dynamics() const;
        LinearPhysicsComponent& dynamics();

        const MaterialRef& material() const;
        MaterialRef& material();

        const LinearSubMaterialRef& sub_material() const;
        LinearSubMaterialRef& sub_material();

        const LinearPhysicsProperties& properties() const;
        LinearPhysicsProperties& properties();

    private:
        void handle_collides(const col2d::CollisionEventData& data) const;
    };

    using LinearBodyRef = SmartReference<LinearBody>;
}
