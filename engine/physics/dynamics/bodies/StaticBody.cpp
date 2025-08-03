#include "StaticBody.h"

#include "core/context/Collision.h"

namespace oly::physics
{
	StaticBody::StaticBody()
		: RigidBody()
	{
	}

	StaticBody::StaticBody(const StaticBody& other)
		: RigidBody(other), dynamics(other.dynamics)
	{
		bind_all();
	}

	StaticBody::StaticBody(StaticBody&& other) noexcept
		: RigidBody(std::move(other)), dynamics(std::move(other.dynamics))
	{
		bind_all();
	}

	StaticBody::~StaticBody()
	{
		unbind_all();
	}

	void StaticBody::physics_pre_tick()
	{
		dynamics.pre_tick(transformer.global());
	}

	void StaticBody::physics_post_tick()
	{
		dynamics.post_tick();
		transformer.set_global(Transform2D{ .position = dynamics.get_state().position, .rotation = dynamics.get_state().rotation, .scale = transformer.get_local().scale }.matrix());
	}

	void StaticBody::handle_overlaps(const col2d::OverlapEventData& data) const
	{
		if (data.phase & (col2d::Phase::STARTED | col2d::Phase::ONGOING))
			if (const RigidBody* other = rigid_body(*data.passive_collider))
				if (other != this)
					dynamics.add_collision({}, {}, dynamics_of(*other));
	}

	void StaticBody::bind(const col2d::Collider& collider) const
	{
		context::collision_dispatcher().register_handler(collider.cref(), &StaticBody::handle_overlaps, cref());
	}

	void StaticBody::unbind(const col2d::Collider& collider) const
	{
		context::collision_dispatcher().unregister_handler(collider.cref(), &StaticBody::handle_overlaps, cref());
	}
}
