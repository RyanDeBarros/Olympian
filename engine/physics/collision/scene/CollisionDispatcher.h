#pragma once

#include "physics/collision/scene/CollisionTree.h"
#include "core/platform/Events.h"

#include <unordered_set>

namespace oly::col2d
{
	class CollisionDispatcher
	{
		// TODO don't use direct pointer, since if Collider is destroyed, handler maps will store dangling pointers
		std::unordered_map<ConstSoftReference<Collider>, std::unique_ptr<EventHandler<OverlapResult>>> overlap_handlers;
		std::unordered_map<ConstSoftReference<Collider>, std::unique_ptr<EventHandler<CollisionResult>>> collision_handlers;
		std::unordered_map<ConstSoftReference<Collider>, std::unique_ptr<EventHandler<ContactResult>>> contact_handlers;

		CollisionTree tree;

	public:
		CollisionDispatcher(const math::Rect2D bounds, const glm::uvec2 degree = { 2, 2 }, const size_t cell_capacity = 4) : tree(bounds, degree, cell_capacity) {}

		void register_handler(const ConstSoftReference<Collider>& collider, std::unique_ptr<EventHandler<OverlapResult>>&& handler) { overlap_handlers[collider] = std::move(handler); }
		void register_handler(ConstSoftReference<Collider>&& collider, std::unique_ptr<EventHandler<OverlapResult>>&& handler) { overlap_handlers[std::move(collider)] = std::move(handler); }
		void register_handler(const ConstSoftReference<Collider>& collider, std::unique_ptr<EventHandler<CollisionResult>>&& handler) { collision_handlers[collider] = std::move(handler); }
		void register_handler(ConstSoftReference<Collider>&& collider, std::unique_ptr<EventHandler<CollisionResult>>&& handler) { collision_handlers[std::move(collider)] = std::move(handler); }
		void register_handler(const ConstSoftReference<Collider>& collider, std::unique_ptr<EventHandler<ContactResult>>&& handler) { contact_handlers[collider] = std::move(handler); }
		void register_handler(ConstSoftReference<Collider>&& collider, std::unique_ptr<EventHandler<ContactResult>>&& handler) { contact_handlers[std::move(collider)] = std::move(handler); }
		void unregister_overlap_handler(const ConstSoftReference<Collider>& collider) { overlap_handlers.erase(collider); }
		void unregister_collision_handler(const ConstSoftReference<Collider>& collider) { collision_handlers.erase(collider); }
		void unregister_contact_handler(const ConstSoftReference<Collider>& collider) { contact_handlers.erase(collider); }

		// TODO just expose tree methods, not tree itself
		CollisionTree& ref_tree() { return tree; }

		void poll();
	};
}
