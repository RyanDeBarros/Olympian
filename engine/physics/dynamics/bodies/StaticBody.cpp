#include "StaticBody.h"

#include "core/context/Collision.h"

namespace oly::physics
{
	StaticBody::StaticBody()
		: RigidBody()
	{
	}

	StaticBody::StaticBody(StaticBody&& o) noexcept
		: RigidBody(std::move(o))
        , dynamics_(std::move(o.dynamics_))
        , delegator(std::move(o.delegator))
	{
		bind_all();
	}

	StaticBody::~StaticBody()
	{
		unbind_all();
	}

    State StaticBody::state() const
    {
        return dynamics_.get_state();
    }

    bool StaticBody::is_colliding() const
    {
        return dynamics_.is_colliding();
    }

	void StaticBody::physics_pre_tick()
	{
		dynamics_.pre_tick(transformer_->global());
	}

	void StaticBody::physics_post_tick()
	{
		dynamics_.post_tick();
		transformer_->set_global(Transform2D{
            .position = dynamics_.get_state().position,
            .rotation = dynamics_.get_state().rotation,
            .scale = transformer_->get_local().scale }.matrix());
	}

	void StaticBody::bind(const col2d::Collider& collider) const
	{
		col2d::CollisionController::bind(collider, &StaticBody::handle_overlaps);
	}

	void StaticBody::unbind(const col2d::Collider& collider) const
	{
		col2d::CollisionController::unbind(collider, &StaticBody::handle_overlaps);
	}

    const DynamicsComponent& StaticBody::get_dynamics() const
    {
        return dynamics_;
    }

    const DynamicsComponent& StaticBody::dynamics() const
    {
        return dynamics_;
    }

    DynamicsComponent& StaticBody::dynamics()
    {
        return dynamics_;
    }

    const MaterialRef& StaticBody::material() const
    {
        return dynamics_.material;
    }
    
    MaterialRef& StaticBody::material()
    {
        return dynamics_.material;
    }

    void StaticBody::handle_overlaps(const col2d::OverlapEventData& data) const
    {
        if (data.phase & (col2d::Phase::Started | col2d::Phase::Ongoing))
        {
            const RigidBody* other = rigid_body(data.passive_collider);
            if (other && other != this)
                dynamics_.add_collision({}, {}, dynamics_of(*other));
        }

        delegator.emit(data);
    }
}
