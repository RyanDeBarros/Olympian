#include "LinearBody.h"

namespace oly::physics
{
	LinearBody::LinearBody()
		: RigidBody()
	{
	}

	LinearBody::LinearBody(const LinearBody& other)
		: RigidBody(other), dynamics(other.dynamics)
	{
		bind_all();
	}

	LinearBody::LinearBody(LinearBody&& other) noexcept
		: RigidBody(std::move(other)), dynamics(std::move(other.dynamics))
	{
		bind_all();
	}

	LinearBody::~LinearBody()
	{
		unbind_all();
	}

	void LinearBody::physics_pre_tick()
	{
		dynamics.pre_tick(transformer.global());
	}

	void LinearBody::physics_post_tick()
	{
		dynamics.post_tick();
		transformer.set_global(Transform2D{ .position = dynamics.get_state().position, .rotation = dynamics.get_state().rotation, .scale = transformer.get_local().scale }.matrix());
	}

	void LinearBody::handle_collides(const col2d::CollisionEventData& data) const
	{
		if (data.phase & (col2d::Phase::STARTED | col2d::Phase::ONGOING))
			if (const RigidBody* other = rigid_body(data.passive_collider))
				if (other != this)
					dynamics.add_collision(data.mtv(), {}, dynamics_of(*other));
	}

	void LinearBody::bind(const col2d::Collider& collider) const
	{
		col2d::CollisionController::bind(collider, &LinearBody::handle_collides);
	}

	void LinearBody::unbind(const col2d::Collider& collider) const
	{
		col2d::CollisionController::unbind(collider, &LinearBody::handle_collides);
	}
}
