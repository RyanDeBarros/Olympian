#pragma once

#include "physics/dynamics/bodies/RigidBody.h"

#include "physics/dynamics/components/KinematicPhysicsComponent.h"

#include <imp/action.hpp>

namespace oly::physics
{
    class KinematicBody
        : public RigidBody
    {
    private:
        KinematicPhysicsComponent dynamics_;

    public:
        imp::action_delegator<col2d::ContactEventData> delegator;

        KinematicBody();
        KinematicBody(const KinematicBody&) = delete;
        KinematicBody(KinematicBody&&) noexcept;
        ~KinematicBody();

        KinematicBody& operator=(const KinematicBody&) = delete;
        KinematicBody& operator=(KinematicBody&&) noexcept;

    public:
        State state() const override;
        bool is_colliding() const override;

    protected:
        void physics_pre_tick() override;
        void physics_post_tick() override;

        void bind(const col2d::Collider& collider) const override;
        void unbind(const col2d::Collider& collider) const override;

        const DynamicsComponent& get_dynamics() const override;

    public:
        const KinematicPhysicsComponent& dynamics() const;
        KinematicPhysicsComponent& dynamics();

        const MaterialRef& material() const;
        MaterialRef& material();

        const KinematicSubMaterialRef& sub_material() const;
        KinematicSubMaterialRef& sub_material();

        const KinematicPhysicsProperties& properties() const;
        KinematicPhysicsProperties& properties();

    private:
        void handle_contacts(const col2d::ContactEventData& data) const;
    };

    using KinematicBodyRef = SmartReference<KinematicBody>;
}
