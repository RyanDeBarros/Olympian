#include "RigidBody.h"

namespace oly::physics
{
	RigidBody::RigidBody()
	{
		internal::RigidBodyManager::instance().rigid_bodies.insert(this);
	}

	RigidBody::RigidBody(const RigidBody& other)
		: colliders(other.colliders), transformer(other.transformer)
	{
		internal::RigidBodyManager::instance().rigid_bodies.insert(this);
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			it->rigid_body = this;
			it->set_transformer().attach_parent(&transformer);
		}
	}
	
	RigidBody::RigidBody(RigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), transformer(std::move(other.transformer))
	{
		internal::RigidBodyManager::instance().rigid_bodies.insert(this);
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			it->rigid_body = this;
			it->set_transformer().attach_parent(&transformer);
			other.unbind(*it);
		}
	}
	
	RigidBody::~RigidBody()
	{
		internal::RigidBodyManager::instance().rigid_bodies.erase(this);
		colliders.clear();
	}
	
	RigidBody& RigidBody::operator=(const RigidBody& other)
	{
		if (this != &other)
		{
			clear_colliders();
			colliders = other.colliders;
			transformer = other.transformer;
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
			{
				it->rigid_body = this;
				it->set_transformer().attach_parent(&transformer);
				bind(*it);
			}
		}
		return *this;
	}
	
	RigidBody& RigidBody::operator=(RigidBody&& other) noexcept
	{
		if (this != &other)
		{
			clear_colliders();
			colliders = std::move(other.colliders);
			transformer = std::move(other.transformer);
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
			{
				it->rigid_body = this;
				it->set_transformer().attach_parent(&transformer);
				bind(*it);
				other.unbind(*it);
			}
		}
		return *this;
	}
	
	col2d::Collider& RigidBody::add_collider(col2d::Collider&& collider)
	{
		if (collider.rigid_body)
			collider.rigid_body->remove_collider(collider);
		col2d::Collider& c = colliders.emplace_back(std::move(collider));
		c.rigid_body = this;
		c.set_transformer().attach_parent(&transformer);
		c.handles.attach();
		bind(c);
		return c;
	}

	void RigidBody::erase_collider(size_t i)
	{
		unbind(colliders[i]);
		colliders.erase(colliders.begin() + i);
	}

	void RigidBody::remove_collider(const col2d::Collider& collider)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			if (&*it == &collider)
			{
				unbind(*it);
				colliders.erase(it);
				return;
			}
		}
	}

	void RigidBody::clear_colliders()
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			unbind(*it);
		colliders.clear();
	}

	const col2d::Collider& RigidBody::collider(size_t i) const
	{
		return colliders[i];
	}

	col2d::Collider& RigidBody::collider(size_t i)
	{
		return colliders[i];
	}

	debug::CollisionView RigidBody::collision_view(size_t i, glm::vec4 color) const
	{
		return colliders[i].collision_view(color);
	}

	void RigidBody::update_view(size_t i, debug::CollisionView& view, glm::vec4 color) const
	{
		colliders[i].update_view(view, color);
	}

	void RigidBody::update_view(size_t i, debug::CollisionView& view) const
	{
		colliders[i].update_view(view);
	}

	void RigidBody::bind_all() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			bind(*it);
	}

	void RigidBody::unbind_all() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			unbind(*it);
	}

	const RigidBody* RigidBody::rigid_body(const col2d::Collider& collider) const
	{
		return collider.rigid_body;
	}

	void internal::RigidBodyManager::on_tick() const
	{
		for (RigidBody* rigid_body : rigid_bodies)
			rigid_body->physics_pre_tick();
		for (RigidBody* rigid_body : rigid_bodies)
			rigid_body->physics_post_tick();
	}
}
