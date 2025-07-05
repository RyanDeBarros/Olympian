#include "RigidBody.h"

#include "core/base/Context.h"

namespace oly::physics
{
	SimpleRigidBody::SimpleRigidBody(const SimpleRigidBody& other)
		: colliders(other.colliders), transformer(other.transformer)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().register_handler(*it, &SimpleRigidBody::handle_collides, ref());
	}
	
	SimpleRigidBody::SimpleRigidBody(SimpleRigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), transformer(std::move(other.transformer))
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			context::collision_dispatcher().register_handler(*it, &SimpleRigidBody::handle_collides, ref());
			context::collision_dispatcher().unregister_handler(*it, &SimpleRigidBody::handle_collides, other.ref());
		}
	}

	SimpleRigidBody::~SimpleRigidBody()
	{
		clear_colliders();
	}

	SimpleRigidBody& SimpleRigidBody::operator=(const SimpleRigidBody& other)
	{
		if (this != &other)
		{
			clear_colliders();
			colliders = other.colliders;
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
				context::collision_dispatcher().register_handler(*it, &SimpleRigidBody::handle_collides, ref());
			
			transformer = other.transformer;
		}
		return *this;
	}

	SimpleRigidBody& SimpleRigidBody::operator=(SimpleRigidBody&& other) noexcept
	{
		if (this != &other)
		{
			clear_colliders();
			colliders = std::move(other.colliders);
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
			{
				context::collision_dispatcher().register_handler(*it, &SimpleRigidBody::handle_collides, ref());
				context::collision_dispatcher().unregister_handler(*it, &SimpleRigidBody::handle_collides, other.ref());
			}

			transformer = std::move(other.transformer);
		}
		return *this;
	}

	void SimpleRigidBody::bind_collider(const ConstSoftReference<col2d::Collider>& collider)
	{
		if (colliders.insert(collider))
			context::collision_dispatcher().register_handler(collider, &SimpleRigidBody::handle_collides, ref());
	}

	void SimpleRigidBody::unbind_collider(const ConstSoftReference<col2d::Collider>& collider)
	{
		if (colliders.erase(collider))
			context::collision_dispatcher().unregister_handler(collider, &SimpleRigidBody::handle_collides, ref());
	}

	void SimpleRigidBody::clear_colliders()
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().unregister_handler(*it, &SimpleRigidBody::handle_collides, ref());
		colliders.clear();
	}

	void SimpleRigidBody::handle_collides(const col2d::CollisionEventData& data)
	{
		// TODO
	}

	RigidBody::RigidBody(const RigidBody& other)
		: colliders(other.colliders), transformer(other.transformer)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().register_handler(*it, &RigidBody::handle_contacts, ref());
	}
	
	RigidBody::RigidBody(RigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), transformer(std::move(other.transformer))
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

			transformer = other.transformer;
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

			transformer = std::move(other.transformer);
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
	
	void RigidBody::handle_contacts(const col2d::ContactEventData& data)
	{
		// TODO
	}
}
