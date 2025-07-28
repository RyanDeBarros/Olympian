#include "RigidBody.h"

#include "core/base/Context.h"

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
			(*it)->rigid_body = this;
			(*it)->set_transformer().attach_parent(&transformer);
		}
	}
	
	RigidBody::RigidBody(RigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), transformer(std::move(other.transformer))
	{
		internal::RigidBodyManager::instance().rigid_bodies.insert(this);
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			(*it)->rigid_body = this;
			(*it)->set_transformer().attach_parent(&transformer);
			other.unbind(**it);
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
				(*it)->rigid_body = this;
				(*it)->set_transformer().attach_parent(&transformer);
				bind(**it);
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
				(*it)->rigid_body = this;
				(*it)->set_transformer().attach_parent(&transformer);
				bind(**it);
				other.unbind(**it);
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
		bind(*c);
		return c->ref();
	}

	void RigidBody::erase_collider(size_t i)
	{
		unbind(*colliders[i]);
		colliders.erase(colliders.begin() + i);
	}

	void RigidBody::remove_collider(const SoftReference<col2d::Collider>& collider)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			if (it->get() == collider.get())
			{
				unbind(**it);
				colliders.erase(it);
				return;
			}
		}
	}

	void RigidBody::clear_colliders()
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			unbind(**it);
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

	void RigidBody::bind_all() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			bind(**it);
	}
	void RigidBody::unbind_all() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			unbind(**it);
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
			if (const RigidBody* other = rigid_body(*data.passive_collider))
				if (other != this)
					dynamics.add_collision(data.mtv(), {}, dynamics_of(*other));
	}

	void LinearBody::bind(const col2d::Collider& collider) const
	{
		context::collision_dispatcher().register_handler(collider.cref(), &LinearBody::handle_collides, cref());
	}

	void LinearBody::unbind(const col2d::Collider& collider) const
	{
		context::collision_dispatcher().unregister_handler(collider.cref(), &LinearBody::handle_collides, cref());
	}

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
			if (const RigidBody* other = rigid_body(*data.passive_collider))
				if (other != this)
					dynamics.add_collision(data.active_contact.impulse, data.active_contact.position - dynamics.get_state().position, dynamics_of(*other));
	}

	void KinematicBody::bind(const col2d::Collider& collider) const
	{
		context::collision_dispatcher().register_handler(collider.cref(), &KinematicBody::handle_contacts, cref());
	}

	void KinematicBody::unbind(const col2d::Collider& collider) const
	{
		context::collision_dispatcher().unregister_handler(collider.cref(), &KinematicBody::handle_contacts, cref());
	}
}
