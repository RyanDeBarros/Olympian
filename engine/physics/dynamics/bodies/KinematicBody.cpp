#include "KinematicBody.h"

namespace oly::physics
{
    KinematicBody::KinematicBody()
        : RigidBody()
    {
    }

    KinematicBody::KinematicBody(KinematicBody&& o) noexcept
        : RigidBody(std::move(o))
        , dynamics_(std::move(o.dynamics_))
        , delegator(std::move(o.delegator))
    {
        bind_all();
    }

    KinematicBody::~KinematicBody()
    {
        unbind_all();
    }

    KinematicBody& KinematicBody::operator=(KinematicBody&& o) noexcept
    {
        if (this != &o)
        {
            RigidBody::operator=(std::move(o));

            dynamics_ = std::move(o.dynamics_);
            delegator = std::move(o.delegator);
        }

        return *this;
    }

    State KinematicBody::state() const
    {
        return dynamics_.get_state();
    }

    bool KinematicBody::is_colliding() const
    {
        return dynamics_.is_colliding();
    }

    void KinematicBody::physics_pre_tick()
    {
        dynamics_.pre_tick(transformer_->global());
    }

    void KinematicBody::physics_post_tick()
    {
        dynamics_.post_tick();
        transformer_->set_global(Transform2D{
            .position = dynamics_.get_state().position,
            .rotation = dynamics_.get_state().rotation,
            .scale = transformer_->get_local().scale }.matrix());
    }

    void KinematicBody::bind(const col2d::Collider& collider) const
    {
        col2d::CollisionController::bind(collider, &KinematicBody::handle_contacts);
    }

    void KinematicBody::unbind(const col2d::Collider& collider) const
    {
        col2d::CollisionController::unbind(collider, &KinematicBody::handle_contacts);
    }

    const DynamicsComponent& KinematicBody::get_dynamics() const
    {
        return dynamics_;
    }

    const KinematicPhysicsComponent& KinematicBody::dynamics() const
    {
        return dynamics_;
    }

    KinematicPhysicsComponent& KinematicBody::dynamics()
    {
        return dynamics_;
    }

    const MaterialRef& KinematicBody::material() const
    {
        return dynamics_.material;
    }

    MaterialRef& KinematicBody::material()
    {
        return dynamics_.material;
    }

    const KinematicSubMaterialRef& KinematicBody::sub_material() const
    {
        return dynamics_.submaterial;
    }

    KinematicSubMaterialRef& KinematicBody::sub_material()
    {
        return dynamics_.submaterial;
    }

    const KinematicPhysicsProperties& KinematicBody::properties() const
    {
        return dynamics_.properties;
    }

    KinematicPhysicsProperties& KinematicBody::properties()
    {
        return dynamics_.properties;
    }

    void KinematicBody::handle_contacts(const col2d::ContactEventData& data) const
    {
        if (data.phase & (col2d::Phase::Started | col2d::Phase::Ongoing))
        {
            const RigidBody* other = rigid_body(data.passive_collider);
            if (other && other != this)
            {
                dynamics_.add_collision(
                    data.active_contact.impulse,
                    data.active_contact.position - dynamics_.get_state().position,
                    dynamics_of(*other));
            }
        }

        delegator.emit(data);
    }
}
