#include "RigidBody.h"

#include "core/base/Context.h"

namespace oly::physics
{
	SimpleRigidBody::SimpleRigidBody(const SimpleRigidBody& other)
		: colliders(other.colliders), _transformer(other._transformer)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().register_handler(*it, &SimpleRigidBody::handle_collides, ref());
	}
	
	SimpleRigidBody::SimpleRigidBody(SimpleRigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), _transformer(std::move(other._transformer))
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
			
			_transformer = other._transformer;
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

			_transformer = std::move(other._transformer);
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
}
