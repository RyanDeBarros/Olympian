#pragma once

#include "physics/collision/scene/CollisionDispatcher.h"

namespace oly::physics
{
	class RigidBody : public col2d::CollisionController
	{
		OLY_COLLISION_CONTROLLER_HEADER(RigidBody);

	public:
		col2d::Collider collider;

		RigidBody();
		RigidBody(col2d::Collider&& collider);
		~RigidBody();

		void attach_tree(size_t context_tree_index = 0);
		void detach_tree(size_t context_tree_index = 0);
		bool tree_is_attached(size_t context_tree_index = 0) const;
		void clear_trees();

	private:
		void register_handlers();
		void unregister_handlers();
		void handle_overlaps(const col2d::OverlapEventData& data);
		void handle_collides(const col2d::CollisionEventData& data);
		void handle_contacts(const col2d::ContactEventData& data);
	};
}
