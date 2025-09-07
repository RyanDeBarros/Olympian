#include "KinematicBody.h"

namespace oly::physics
{
	KinematicBody::KinematicBody()
		: RigidBody()
	{
	}

	KinematicBody::KinematicBody(const KinematicBody& other)
		: RigidBody(other), dynamics(other.dynamics)
	{
		bind_all();
	}

	KinematicBody::KinematicBody(KinematicBody&& other) noexcept
		: RigidBody(std::move(other)), dynamics(std::move(other.dynamics))
	{
		bind_all();
	}

	KinematicBody::~KinematicBody()
	{
		unbind_all();
	}

	void KinematicBody::physics_pre_tick()
	{
		dynamics.pre_tick(transformer.global());
	}

	void KinematicBody::physics_post_tick()
	{
		dynamics.post_tick();
		transformer.set_global(Transform2D{ .position = dynamics.get_state().position, .rotation = dynamics.get_state().rotation, .scale = transformer.get_local().scale }.matrix());
	}

	void KinematicBody::handle_contacts(const col2d::ContactEventData& data) const
	{
		if (data.phase & (col2d::Phase::STARTED | col2d::Phase::ONGOING))
			if (const RigidBody* other = rigid_body(data.passive_collider))
				if (other != this)
					dynamics.add_collision(data.active_contact.impulse, data.active_contact.position - dynamics.get_state().position, dynamics_of(*other));
		delegator.emit(data);
	}

	void KinematicBody::bind(const col2d::Collider& collider) const
	{
		col2d::CollisionController::bind(collider, &KinematicBody::handle_contacts);
	}

	void KinematicBody::unbind(const col2d::Collider& collider) const
	{
		col2d::CollisionController::unbind(collider, &KinematicBody::handle_contacts);
	}
}
