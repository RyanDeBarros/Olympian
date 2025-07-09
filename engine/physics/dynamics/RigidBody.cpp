#include "RigidBody.h"

#include "core/base/Context.h"

namespace oly::physics
{
	RigidBody::RigidBody(const RigidBody& other)
		: colliders(other.colliders), dynamics(other.dynamics)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			(*it)->rigid_body = this;
			(*it)->set_transformer().attach_parent(&transformer);
			context::collision_dispatcher().register_handler((*it)->cref(), &RigidBody::handle_contacts, cref());
		}
	}
	
	RigidBody::RigidBody(RigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), dynamics(std::move(other.dynamics))
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			(*it)->rigid_body = this;
			(*it)->set_transformer().attach_parent(&transformer);
			context::collision_dispatcher().register_handler((*it)->cref(), &RigidBody::handle_contacts, cref());
			context::collision_dispatcher().unregister_handler((*it)->cref(), &RigidBody::handle_contacts, other.cref());
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
			{
				(*it)->rigid_body = this;
				(*it)->set_transformer().attach_parent(&transformer);
				context::collision_dispatcher().register_handler((*it)->cref(), &RigidBody::handle_contacts, cref());
			}

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
				(*it)->rigid_body = this;
				(*it)->set_transformer().attach_parent(&transformer);
				context::collision_dispatcher().register_handler((*it)->cref(), &RigidBody::handle_contacts, cref());
				context::collision_dispatcher().unregister_handler((*it)->cref(), &RigidBody::handle_contacts, other.cref());
			}

			dynamics = std::move(other.dynamics);
		}
		return *this;
	}
	
	SoftReference<col2d::Collider> RigidBody::add_collider(col2d::Collider&& collider)
	{
		if (collider.rigid_body)
			collider.rigid_body->remove_collider(collider.ref());
		CopyPtr<col2d::Collider>& c = colliders.emplace_back(std::move(collider));
		c->rigid_body = this;
		c->set_transformer().attach_parent(&transformer);
		c->handles.attach();
		context::collision_dispatcher().register_handler(c->cref(), &RigidBody::handle_contacts, cref());
		return c->ref();
	}

	void RigidBody::erase_collider(size_t i)
	{
		context::collision_dispatcher().unregister_handler(colliders[i]->cref(), &RigidBody::handle_contacts, cref());
		colliders.erase(colliders.begin() + i);
	}

	void RigidBody::remove_collider(const SoftReference<col2d::Collider>& collider)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			if (it->get() == collider.get())
			{
				context::collision_dispatcher().unregister_handler((*it)->cref(), &RigidBody::handle_contacts, cref());
				colliders.erase(it);
			}
		}
	}

	void RigidBody::clear_colliders()
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().unregister_handler((*it)->cref(), &RigidBody::handle_contacts, cref());
		colliders.clear();
	}

	SoftReference<col2d::Collider> RigidBody::collider(size_t i)
	{
		return colliders[i]->ref();
	}

	debug::CollisionView RigidBody::collision_view(size_t i, glm::vec4 color) const
	{
		return colliders[i]->collision_view(color);
	}

	void RigidBody::update_view(size_t i, debug::CollisionView& view, glm::vec4 color) const
	{
		colliders[i]->update_view(view, color);
	}

	void RigidBody::on_tick()
	{
		dynamics.on_tick();
		// TODO this obviously doesn't take into account transformer.local
		transformer.set_global(Transform2D{ .position = dynamics.get_state().position, .rotation = dynamics.get_state().rotation }.matrix());
	}

	// TODO option to not bind handles if dynamics is STATIC. also use handle_collides if only linear motion. define special flag in DynamicsComponent for that.

	void RigidBody::handle_contacts(const col2d::ContactEventData& data) const
	{
		if (data.phase & (col2d::Phase::STARTED | col2d::Phase::ONGOING))
			if (RigidBody* other = data.passive_collider->rigid_body)
				if (other != this)
					dynamics.add_collision(data.active_contact.impulse, data.active_contact.position - dynamics.get_state().position, other->dynamics);
	}
}
