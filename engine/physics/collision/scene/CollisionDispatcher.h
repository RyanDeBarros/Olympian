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
		OLY_SOFT_REFERENCE_BASE_DECLARATION(CollisionController);

	public:
		virtual ~CollisionController() = default;

		using OverlapHandler = void(CollisionController::*)(const OverlapEventData&);
		using OverlapConstHandler = void(CollisionController::*)(const OverlapEventData&) const;
		using CollisionHandler = void(CollisionController::*)(const CollisionEventData&);
		using CollisionConstHandler = void(CollisionController::*)(const CollisionEventData&) const;
		using ContactHandler = void(CollisionController::*)(const ContactEventData&);
		using ContactConstHandler = void(CollisionController::*)(const ContactEventData&) const;
	};

#define OLY_COLLISION_CONTROLLER_HEADER(Class)\
	OLY_SOFT_REFERENCE_PUBLIC(Class)

	class CollisionDispatcher
	{
		struct OverlapHandlerRef
		{
			CollisionController::OverlapHandler handler = nullptr;
			SoftReference<CollisionController> controller = nullptr;
		};

		struct OverlapConstHandlerRef
		{
			CollisionController::OverlapConstHandler handler = nullptr;
			ConstSoftReference<CollisionController> controller = nullptr;
		};

		struct CollisionHandlerRef
		{
			CollisionController::CollisionHandler handler = nullptr;
			SoftReference<CollisionController> controller = nullptr;
		};

		struct CollisionConstHandlerRef
		{
			CollisionController::CollisionConstHandler handler = nullptr;
			ConstSoftReference<CollisionController> controller = nullptr;
		};

		struct ContactHandlerRef
		{
			CollisionController::ContactHandler handler = nullptr;
			SoftReference<CollisionController> controller = nullptr;
		};

		struct ContactConstHandlerRef
		{
			CollisionController::ContactConstHandler handler = nullptr;
			ConstSoftReference<CollisionController> controller = nullptr;
		};

		std::unordered_map<ConstSoftReference<Collider>, std::variant<OverlapHandlerRef, OverlapConstHandlerRef>> overlap_handlers;
		std::unordered_map<ConstSoftReference<Collider>, std::variant<CollisionHandlerRef, CollisionConstHandlerRef>> collision_handlers;
		std::unordered_map<ConstSoftReference<Collider>, std::variant<ContactHandlerRef, ContactConstHandlerRef>> contact_handlers;

		CollisionTree tree;

	public:
		CollisionDispatcher(const math::Rect2D bounds, const glm::uvec2 degree = { 2, 2 }, const size_t cell_capacity = 4) : tree(bounds, degree, cell_capacity) {}

		void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::OverlapHandler handler, const SoftReference<CollisionController>& controller)
		{
			overlap_handlers[collider] = OverlapHandlerRef{ handler, controller };
		}
		void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::OverlapConstHandler handler, const ConstSoftReference<CollisionController>& controller)
		{
			overlap_handlers[collider] = OverlapConstHandlerRef{ handler, controller };
		}
		void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::CollisionHandler handler, const SoftReference<CollisionController>& controller)
		{
			collision_handlers[collider] = CollisionHandlerRef{ handler, controller };
		}
		void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::CollisionConstHandler handler, const ConstSoftReference<CollisionController>& controller)
		{
			collision_handlers[collider] = CollisionConstHandlerRef{ handler, controller };
		}
		void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::ContactHandler handler, const SoftReference<CollisionController>& controller)
		{
			contact_handlers[collider] = ContactHandlerRef{ handler, controller };
		}
		void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::ContactConstHandler handler, const ConstSoftReference<CollisionController>& controller)
		{
			contact_handlers[collider] = ContactConstHandlerRef{ handler, controller };
		}

		template<std::derived_from<CollisionController> Controller>
		void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const OverlapEventData&), const SoftReference<Controller>& controller)
		{
			overlap_handlers[collider] = OverlapHandlerRef{ static_cast<CollisionController::OverlapHandler>(handler), controller };
		}
		template<std::derived_from<CollisionController> Controller>
		void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const OverlapEventData&) const, const ConstSoftReference<Controller>& controller)
		{
			overlap_handlers[collider] = OverlapConstHandlerRef{ static_cast<CollisionController::OverlapConstHandler>(handler), controller };
		}
		template<std::derived_from<CollisionController> Controller>
		void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const CollisionEventData&), const SoftReference<Controller>& controller)
		{
			collision_handlers[collider] = CollisionHandlerRef{ static_cast<CollisionController::CollisionHandler>(handler), controller };
		}
		template<std::derived_from<CollisionController> Controller>
		void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const CollisionEventData&) const, const ConstSoftReference<Controller>& controller)
		{
			collision_handlers[collider] = CollisionConstHandlerRef{ static_cast<CollisionController::CollisionConstHandler>(handler), controller };
		}
		template<std::derived_from<CollisionController> Controller>
		void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const ContactEventData&), const SoftReference<Controller>& controller)
		{
			contact_handlers[collider] = ContactHandlerRef{ static_cast<CollisionController::ContactHandler>(handler), controller };
		}
		template<std::derived_from<CollisionController> Controller>
		void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const ContactEventData&) const, const ConstSoftReference<Controller>& controller)
		{
			contact_handlers[collider] = ContactConstHandlerRef{ static_cast<CollisionController::ContactConstHandler>(handler), controller };
		}

		void unregister_overlap_handler(const ConstSoftReference<Collider>& collider) { overlap_handlers.erase(collider); }
		void unregister_collision_handler(const ConstSoftReference<Collider>& collider) { collision_handlers.erase(collider); }
		void unregister_contact_handler(const ConstSoftReference<Collider>& collider) { contact_handlers.erase(collider); }
		void unregister_handlers(const ConstSoftReference<Collider>& collider) { overlap_handlers.erase(collider); collision_handlers.erase(collider); contact_handlers.erase(collider); }

		// TODO just expose tree methods, not tree itself
		CollisionTree& ref_tree() { return tree; }

		// call poll() after all collision objects have moved, but before handling events
		void poll();
		void clean();

		void emit(const Collider& from);
		void emit(const Collider& from, CollisionController::OverlapHandler only_handler, const SoftReference<CollisionController>& only_controller) const;
		void emit(const Collider& from, CollisionController::OverlapConstHandler only_handler, const ConstSoftReference<CollisionController>& only_controller) const;
		void emit(const Collider& from, CollisionController::CollisionHandler only_handler, const SoftReference<CollisionController>& only_controller) const;
		void emit(const Collider& from, CollisionController::CollisionConstHandler only_handler, const ConstSoftReference<CollisionController>& only_controller) const;
		void emit(const Collider& from, CollisionController::ContactHandler only_handler, const SoftReference<CollisionController>& only_controller) const;
		void emit(const Collider& from, CollisionController::ContactConstHandler only_handler, const ConstSoftReference<CollisionController>& only_controller) const;
		
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const OverlapEventData&), const SoftReference<Controller>& only_controller) const
		{
			emit(from, static_cast<CollisionController::OverlapHandler>(only_handler), only_controller);
		}
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const OverlapEventData&) const, const ConstSoftReference<Controller>& only_controller) const
		{
			emit(from, static_cast<CollisionController::OverlapConstHandler>(only_handler), only_controller);
		}
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const CollisionEventData&), const SoftReference<Controller>& only_controller) const
		{
			emit(from, static_cast<CollisionController::CollisionHandler>(only_handler), only_controller);
		}
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const CollisionEventData&) const, const ConstSoftReference<Controller>& only_controller) const
		{
			emit(from, static_cast<CollisionController::CollisionConstHandler>(only_handler), only_controller);
		}
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const ContactEventData&), const SoftReference<Controller>& only_controller) const
		{
			emit(from, static_cast<CollisionController::ContactHandler>(only_handler), only_controller);
		}
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const ContactEventData&) const, const ConstSoftReference<Controller>& only_controller) const
		{
			emit(from, static_cast<CollisionController::ContactConstHandler>(only_handler), only_controller);
		}
	};
}
