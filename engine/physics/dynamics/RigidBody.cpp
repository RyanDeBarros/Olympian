#include "RigidBody.h"

#include "core/base/Context.h"

namespace oly::physics
{
	RigidBody::RigidBody(const RigidBody& other)
		: colliders(other.colliders), transformer(other.transformer), dynamics(other.dynamics)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			(*it)->rigid_body = this;
			(*it)->set_transformer().attach_parent(&transformer);
			bind_by_flag(**it);
		}
	}
	
	RigidBody::RigidBody(RigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), transformer(std::move(other.transformer)), dynamics(std::move(other.dynamics))
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			(*it)->rigid_body = this;
			(*it)->set_transformer().attach_parent(&transformer);
			bind_by_flag(**it);
			other.unbind_by_flag(**it);
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
			transformer = other.transformer;
			dynamics = other.dynamics;
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
			{
				(*it)->rigid_body = this;
				(*it)->set_transformer().attach_parent(&transformer);
				bind_by_flag(**it);
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
			dynamics = std::move(other.dynamics);
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
			{
				(*it)->rigid_body = this;
				(*it)->set_transformer().attach_parent(&transformer);
				bind_by_flag(**it);
				other.unbind_by_flag(**it);
			}
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
		bind_by_flag(*c);
		return c->ref();
	}

	void RigidBody::erase_collider(size_t i)
	{
		unbind_by_flag(*colliders[i]);
		colliders.erase(colliders.begin() + i);
	}

	void RigidBody::remove_collider(const SoftReference<col2d::Collider>& collider)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			if (it->get() == collider.get())
			{
				unbind_by_flag(**it);
				colliders.erase(it);
				return;
			}
		}
	}

	void RigidBody::clear_colliders()
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			unbind_by_flag(**it);
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

	void RigidBody::update_view(size_t i, debug::CollisionView& view) const
	{
		colliders[i]->update_view(view);
	}

	void RigidBody::on_tick()
	{
		dynamics.sync_state(transformer.global());
		dynamics.on_tick();
		transformer.set_global(Transform2D{ .position = dynamics.get_state().position, .rotation = dynamics.get_state().rotation, .scale = transformer.get_local().scale }.matrix());
	}

	void RigidBody::set_flag(DynamicsComponent::Flag flag)
	{
		if (dynamics.flag == DynamicsComponent::Flag::STATIC)
		{
			if (flag == DynamicsComponent::Flag::KINEMATIC)
				bind_contacts_handler();
			else if (flag == DynamicsComponent::Flag::LINEAR)
				bind_collides_handler();
		}
		else if (dynamics.flag == DynamicsComponent::Flag::KINEMATIC)
		{
			if (flag == DynamicsComponent::Flag::STATIC)
				unbind_contacts_handler();
			else if (flag == DynamicsComponent::Flag::LINEAR)
			{
				unbind_contacts_handler();
				bind_collides_handler();
			}
		}
		else if (dynamics.flag == DynamicsComponent::Flag::LINEAR)
		{
			if (flag == DynamicsComponent::Flag::STATIC)
				unbind_collides_handler();
			else if (flag == DynamicsComponent::Flag::KINEMATIC)
			{
				unbind_collides_handler();
				bind_contacts_handler();
			}
		}
		dynamics.flag = flag;
	}

	void RigidBody::handle_collides(const col2d::CollisionEventData& data) const
	{
		if (data.phase & (col2d::Phase::STARTED | col2d::Phase::ONGOING))
			if (RigidBody* other = data.passive_collider->rigid_body)
				if (other != this)
					dynamics.add_collision(data.mtv(), {}, other->dynamics);
	}

	void RigidBody::handle_contacts(const col2d::ContactEventData& data) const
	{
		if (data.phase & (col2d::Phase::STARTED | col2d::Phase::ONGOING))
			if (RigidBody* other = data.passive_collider->rigid_body)
				if (other != this)
					dynamics.add_collision(data.active_contact.impulse, data.active_contact.position - dynamics.get_state().position, other->dynamics);
	}

	void RigidBody::bind_collides_handler() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().register_handler((*it)->cref(), &RigidBody::handle_collides, cref());
	}

	void RigidBody::bind_contacts_handler() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().register_handler((*it)->cref(), &RigidBody::handle_contacts, cref());
	}

	void RigidBody::unbind_collides_handler() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().unregister_handler((*it)->cref(), &RigidBody::handle_collides, cref());
	}

	void RigidBody::unbind_contacts_handler() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			context::collision_dispatcher().unregister_handler((*it)->cref(), &RigidBody::handle_contacts, cref());
	}

	void RigidBody::bind_by_flag(const col2d::Collider& collider) const
	{
		if (dynamics.flag == DynamicsComponent::Flag::KINEMATIC)
			context::collision_dispatcher().register_handler(collider.cref(), &RigidBody::handle_contacts, cref());
		else if (dynamics.flag == DynamicsComponent::Flag::LINEAR)
			context::collision_dispatcher().register_handler(collider.cref(), &RigidBody::handle_collides, cref());
	}
	
	void RigidBody::unbind_by_flag(const col2d::Collider& collider) const
	{
		if (dynamics.flag == DynamicsComponent::Flag::KINEMATIC)
			context::collision_dispatcher().unregister_handler(collider.cref(), &RigidBody::handle_contacts, cref());
		else if (dynamics.flag == DynamicsComponent::Flag::LINEAR)
			context::collision_dispatcher().unregister_handler(collider.cref(), &RigidBody::handle_collides, cref());
	}
}
