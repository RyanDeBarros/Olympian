#include "LinearBody.h"

namespace oly::physics
{
	LinearBody::LinearBody()
		: RigidBody()
	{
	}

	LinearBody::LinearBody(LinearBody&& o) noexcept
		: RigidBody(std::move(o))
        , dynamics_(std::move(o.dynamics_))
        , delegator(std::move(o.delegator))
	{
		bind_all();
	}

	LinearBody::~LinearBody()
	{
		unbind_all();
	}

    LinearBody& LinearBody::operator=(LinearBody&& o) noexcept
    {
        if (this != &o)
        {
            LinearBody::operator=(std::move(o));
            
            dynamics_ = std::move(o.dynamics_);
            delegator = std::move(o.delegator);
        }

        return *this;
    }

    State LinearBody::state() const
    {
        return dynamics_.get_state();
    }

    bool LinearBody::is_colliding() const
    {
        return dynamics_.is_colliding();
    }

	void LinearBody::physics_pre_tick()
	{
		dynamics_.pre_tick(transformer->global());
	}

	void LinearBody::physics_post_tick()
	{
		dynamics_.post_tick();
		transformer->set_global(Transform2D{
            .position = dynamics_.get_state().position,
            .rotation = dynamics_.get_state().rotation,
            .scale = transformer->get_local().scale
        }.matrix());
	}

	void LinearBody::bind(const col2d::Collider& collider) const
	{
		col2d::CollisionController::bind(collider, &LinearBody::handle_collides);
	}

	void LinearBody::unbind(const col2d::Collider& collider) const
	{
		col2d::CollisionController::unbind(collider, &LinearBody::handle_collides);
	}

    const DynamicsComponent& LinearBody::get_dynamics() const
    {
        return dynamics_;
    }

    const LinearPhysicsComponent& LinearBody::dynamics() const
    {
        return dynamics_;
    }
    
    LinearPhysicsComponent& LinearBody::dynamics()
    {
        return dynamics_;
    }

    const MaterialRef& LinearBody::material() const
    {
        return dynamics_.material;
    }
    
    MaterialRef& LinearBody::material()
    {
        return dynamics_.material;
    }

    const LinearSubMaterialRef& LinearBody::sub_material() const
    {
        return dynamics_.submaterial;
    }
    
    LinearSubMaterialRef& LinearBody::sub_material()
    {
        return dynamics_.submaterial;
    }

    const LinearPhysicsProperties& LinearBody::properties() const
    {
        return dynamics_.properties;
    }
    
    LinearPhysicsProperties& LinearBody::properties()
    {
        return dynamics_.properties;
    }

    void LinearBody::handle_collides(const col2d::CollisionEventData& data) const
    {
        if (data.phase & (col2d::Phase::Started | col2d::Phase::Ongoing))
        {
            const RigidBody* other = rigid_body(data.passive_collider);
            if (other && other != this)
                dynamics_.add_collision(data.mtv(), {}, dynamics_of(*other));
        }

        delegator.emit(data);
    }
}
