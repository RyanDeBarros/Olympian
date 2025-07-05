#include "RigidBody.h"

#include "core/base/Context.h"

namespace oly::physics
{
	RigidBody::RigidBody(const RigidBody& other)
		: colliders(other.colliders), dynamics(other.dynamics)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().register_handler(*it, &RigidBody::handle_contacts, ref());
	}
	
	RigidBody::RigidBody(RigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), dynamics(std::move(other.dynamics))
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			context::collision_dispatcher().register_handler(*it, &RigidBody::handle_contacts, ref());
			context::collision_dispatcher().unregister_handler(*it, &RigidBody::handle_contacts, other.ref());
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
				context::collision_dispatcher().register_handler(*it, &RigidBody::handle_contacts, ref());

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
				context::collision_dispatcher().register_handler(*it, &RigidBody::handle_contacts, ref());
				context::collision_dispatcher().unregister_handler(*it, &RigidBody::handle_contacts, other.ref());
			}

			dynamics = std::move(other.dynamics);
		}
		return *this;
	}
	
	void RigidBody::bind_collider(const ConstSoftReference<col2d::Collider>& collider)
	{
		if (colliders.insert(collider))
			context::collision_dispatcher().register_handler(collider, &RigidBody::handle_contacts, ref());
	}
	
	void RigidBody::unbind_collider(const ConstSoftReference<col2d::Collider>& collider)
	{
		if (colliders.erase(collider))
			context::collision_dispatcher().unregister_handler(collider, &RigidBody::handle_contacts, ref());
	}
	
	void RigidBody::clear_colliders()
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().unregister_handler(*it, &RigidBody::handle_contacts, ref());
		colliders.clear();
	}

	void RigidBody::on_tick()
	{
		dynamics.on_tick();
		// TODO set dynamics position/rotation in world coordinates -> use parent's inverse matrix to transform them.
	}

	void RigidBody::handle_contacts(const col2d::ContactEventData& data)
	{
		// TODO something more sophisticated, using GreedyMTV.
		// TODO relative position is data.active_contact.position - center. Use lazy center tracking of colliders. Or maybe, contact.position in collision methods should already be relative.
		// TODO should impulse somehow override existing acceleration/force/velocity/impulses to prevent clipping? Could be an optional setting.
		if (data.phase & (col2d::Phase::STARTED | col2d::Phase::ONGOING))
			dynamics.impulses.push_back({ .impulse = data.active_contact.impulse, .relative_position = data.active_contact.position });
	}
}
