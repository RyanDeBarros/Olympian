#pragma once

#include "physics/collision/scene/CollisionTree.h"

#include <unordered_set>

namespace oly::col2d
{
	struct OverlapEventData
	{
		OverlapResult result;
		ConstSoftReference<Collider> active_collider, passive_collider;

		OverlapEventData& invert() { result.invert(); std::swap(active_collider, passive_collider); return *this; }
	};

	struct CollisionEventData
	{
		CollisionResult result;
		ConstSoftReference<Collider> active_collider, passive_collider;

		CollisionEventData& invert() { result.invert(); std::swap(active_collider, passive_collider); return *this; }
	};

	struct ContactEventData
	{
		ContactResult result;
		ConstSoftReference<Collider> active_collider, passive_collider;

		ContactEventData& invert() { result.invert(); std::swap(active_collider, passive_collider); return *this; }
	};

	struct CollisionController
	{
		virtual ~CollisionController() = default;

		using OverlapHandler = void(CollisionController::*)(const OverlapEventData&);
		using CollisionHandler = void(CollisionController::*)(const CollisionEventData&);
		using ContactHandler = void(CollisionController::*)(const ContactEventData&);
	};

	class CollisionDispatcher
	{
		struct OverlapHandlerRef
		{
			CollisionController::OverlapHandler handler;
			SoftReference<CollisionController> controller;
		};

		struct CollisionHandlerRef
		{
			CollisionController::CollisionHandler handler;
			SoftReference<CollisionController> controller;
		};
		
		struct ContactHandlerRef
		{
			CollisionController::ContactHandler handler;
			SoftReference<CollisionController> controller;
		};

		std::unordered_map<ConstSoftReference<Collider>, OverlapHandlerRef> overlap_handlers;
		std::unordered_map<ConstSoftReference<Collider>, CollisionHandlerRef> collision_handlers;
		std::unordered_map<ConstSoftReference<Collider>, ContactHandlerRef> contact_handlers;

		CollisionTree tree;

	public:
		CollisionDispatcher(const math::Rect2D bounds, const glm::uvec2 degree = { 2, 2 }, const size_t cell_capacity = 4) : tree(bounds, degree, cell_capacity) {}

		void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::OverlapHandler handler, const SoftReference<CollisionController>& controller)
		{
			overlap_handlers[collider] = { handler, controller };
		}
		void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::CollisionHandler handler, const SoftReference<CollisionController>& controller)
		{
			collision_handlers[collider] = { handler, controller };
		}
		void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::ContactHandler handler, const SoftReference<CollisionController>& controller)
		{
			contact_handlers[collider] = { handler, controller };
		}

		template<std::derived_from<CollisionController> Controller>
		void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const OverlapEventData&), const SoftReference<Controller>& controller)
		{
			overlap_handlers[collider] = { static_cast<CollisionController::OverlapHandler>(handler), controller };
		}
		template<std::derived_from<CollisionController> Controller>
		void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const CollisionEventData&), const SoftReference<Controller>& controller)
		{
			collision_handlers[collider] = { static_cast<CollisionController::CollisionHandler>(handler), controller };
		}
		template<std::derived_from<CollisionController> Controller>
		void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const ContactEventData&), const SoftReference<Controller>& controller)
		{
			contact_handlers[collider] = { static_cast<CollisionController::ContactHandler>(handler), controller };
		}

		void unregister_overlap_handler(const ConstSoftReference<Collider>& collider) { overlap_handlers.erase(collider); }
		void unregister_collision_handler(const ConstSoftReference<Collider>& collider) { collision_handlers.erase(collider); }
		void unregister_contact_handler(const ConstSoftReference<Collider>& collider) { contact_handlers.erase(collider); }
		void unregister_handlers(const ConstSoftReference<Collider>& collider) { overlap_handlers.erase(collider); collision_handlers.erase(collider); contact_handlers.erase(collider); }

		// TODO just expose tree methods, not tree itself
		CollisionTree& ref_tree() { return tree; }

		void poll();
		void clean();
	};
}
