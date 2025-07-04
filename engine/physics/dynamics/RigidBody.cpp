#include "RigidBody.h"

#include "core/base/Context.h"

namespace oly::physics
{
	RigidBody::RigidBody()
	{
		register_handlers();
	}

	RigidBody::RigidBody(col2d::Collider&& collider)
		: collider(std::move(collider))
	{
		register_handlers();
	}

	RigidBody::~RigidBody()
	{
		unregister_handlers();
	}

	void RigidBody::attach_tree(size_t context_tree_index)
	{
		collider.handles.attach(context_tree_index);
	}

	void RigidBody::detach_tree(size_t context_tree_index)
	{
		collider.handles.detach(context_tree_index);
	}

	bool RigidBody::tree_is_attached(size_t context_tree_index) const
	{
		return collider.handles.is_attached(context_tree_index);
	}

	void RigidBody::clear_trees()
	{
		collider.handles.clear();
	}
	
	void RigidBody::register_handlers()
	{
		context::collision_dispatcher().register_handler(collider.ref(), &RigidBody::handle_overlaps, ref());
		context::collision_dispatcher().register_handler(collider.ref(), &RigidBody::handle_collides, ref());
		context::collision_dispatcher().register_handler(collider.ref(), &RigidBody::handle_contacts, ref());
	}

	void RigidBody::unregister_handlers()
	{
		context::collision_dispatcher().unregister_handlers(collider.ref());
	}

	void RigidBody::handle_overlaps(const col2d::OverlapEventData& data)
	{
		// TODO
	}
	
	void RigidBody::handle_collides(const col2d::CollisionEventData& data)
	{
		// TODO
	}
	
	void RigidBody::handle_contacts(const col2d::ContactEventData& data)
	{
		// TODO
	}
}
