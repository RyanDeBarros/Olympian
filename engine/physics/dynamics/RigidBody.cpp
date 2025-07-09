#include "RigidBody.h"

#include "core/base/Context.h"

namespace oly::physics
{
	RigidBody::RigidBody(const RigidBody& other)
		: colliders(other.colliders), dynamics(other.dynamics)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().register_handler(*it, &RigidBody::handle_contacts, cref());
	}
	
	RigidBody::RigidBody(RigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), dynamics(std::move(other.dynamics))
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			context::collision_dispatcher().register_handler(*it, &RigidBody::handle_contacts, cref());
			context::collision_dispatcher().unregister_handler(*it, &RigidBody::handle_contacts, other.cref());
		}
	}
	
	RigidBody::~RigidBody()
	{
		clear_colliders();
	}
	
	RigidBody& RigidBody::operator=(const RigidBody& other)
	{
		if (this != &other)
		{
			clear_colliders();
			colliders = other.colliders;
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
				context::collision_dispatcher().register_handler(*it, &RigidBody::handle_contacts, cref());

			dynamics = other.dynamics;
		}
		return *this;
	}
	
	RigidBody& RigidBody::operator=(RigidBody&& other) noexcept
	{
		if (this != &other)
		{
			clear_colliders();
			colliders = std::move(other.colliders);
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
			{
				context::collision_dispatcher().register_handler(*it, &RigidBody::handle_contacts, cref());
				context::collision_dispatcher().unregister_handler(*it, &RigidBody::handle_contacts, other.cref());
			}

			dynamics = std::move(other.dynamics);
		}
		return *this;
	}
	
	void RigidBody::bind_collider(const ConstSoftReference<col2d::Collider>& collider)
	{
		if (colliders.insert(collider))
			context::collision_dispatcher().register_handler(collider, &RigidBody::handle_contacts, cref());
	}
	
	void RigidBody::unbind_collider(const ConstSoftReference<col2d::Collider>& collider)
	{
		if (colliders.erase(collider))
			context::collision_dispatcher().unregister_handler(collider, &RigidBody::handle_contacts, cref());
	}
	
	void RigidBody::clear_colliders()
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().unregister_handler(*it, &RigidBody::handle_contacts, cref());
		colliders.clear();
	}

	void RigidBody::on_tick()
	{
		dynamics.on_tick();
		transformer.set_global(Transform2D{ .position = dynamics.get_state().position, .rotation = dynamics.get_state().rotation }.matrix());
	}

	void RigidBody::handle_contacts(const col2d::ContactEventData& data) const
	{
		if (data.phase & (col2d::Phase::STARTED | col2d::Phase::ONGOING))
			if (RigidBody* other = data.passive_collider->rigid_body)
				dynamics.add_collision(data.active_contact.impulse, data.active_contact.position - dynamics.get_state().position, other->dynamics);
	}
}
