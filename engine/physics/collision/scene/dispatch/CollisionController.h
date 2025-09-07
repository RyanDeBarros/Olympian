#pragma once

#include "physics/collision/scene/colliders/Collider.h"

namespace oly::col2d
{
	struct OverlapEventData;
	struct CollisionEventData;
	struct ContactEventData;

	struct CollisionController
	{
		CollisionController();
		CollisionController(const CollisionController&) = delete;
		CollisionController(CollisionController&&) noexcept = delete;
		virtual ~CollisionController();

		using OverlapHandler = void(CollisionController::*)(const OverlapEventData&);
		using OverlapConstHandler = void(CollisionController::*)(const OverlapEventData&) const;
		using CollisionHandler = void(CollisionController::*)(const CollisionEventData&);
		using CollisionConstHandler = void(CollisionController::*)(const CollisionEventData&) const;
		using ContactHandler = void(CollisionController::*)(const ContactEventData&);
		using ContactConstHandler = void(CollisionController::*)(const ContactEventData&) const;

		void bind(const Collider& collider, OverlapHandler handler);
		void bind(const Collider& collider, OverlapConstHandler handler) const;
		void unbind(const Collider& collider, OverlapHandler handler);
		void unbind(const Collider& collider, OverlapConstHandler handler) const;

		void bind(const Collider& collider, CollisionHandler handler);
		void bind(const Collider& collider, CollisionConstHandler handler) const;
		void unbind(const Collider& collider, CollisionHandler handler);
		void unbind(const Collider& collider, CollisionConstHandler handler) const;

		void bind(const Collider& collider, ContactHandler handler);
		void bind(const Collider& collider, ContactConstHandler handler) const;
		void unbind(const Collider& collider, ContactHandler handler);
		void unbind(const Collider& collider, ContactConstHandler handler) const;

		template<std::derived_from<CollisionController> Controller>
		void bind(const Collider& collider, void(Controller::* handler)(const OverlapEventData&)) { bind(collider, static_cast<OverlapHandler>(handler)); }
		template<std::derived_from<CollisionController> Controller>
		void bind(const Collider& collider, void(Controller::* handler)(const OverlapEventData&) const) const { bind(collider, static_cast<OverlapConstHandler>(handler)); }
		template<std::derived_from<CollisionController> Controller>
		void unbind(const Collider& collider, void(Controller::* handler)(const OverlapEventData&)) { unbind(collider, static_cast<OverlapHandler>(handler)); }
		template<std::derived_from<CollisionController> Controller>
		void unbind(const Collider& collider, void(Controller::* handler)(const OverlapEventData&) const) const { unbind(collider, static_cast<OverlapConstHandler>(handler)); }

		template<std::derived_from<CollisionController> Controller>
		void bind(const Collider& collider, void(Controller::* handler)(const CollisionEventData&)) { bind(collider, static_cast<CollisionHandler>(handler)); }
		template<std::derived_from<CollisionController> Controller>
		void bind(const Collider& collider, void(Controller::* handler)(const CollisionEventData&) const) const { bind(collider, static_cast<CollisionConstHandler>(handler)); }
		template<std::derived_from<CollisionController> Controller>
		void unbind(const Collider& collider, void(Controller::* handler)(const CollisionEventData&)) { unbind(collider, static_cast<CollisionHandler>(handler)); }
		template<std::derived_from<CollisionController> Controller>
		void unbind(const Collider& collider, void(Controller::* handler)(const CollisionEventData&) const) const { unbind(collider, static_cast<CollisionConstHandler>(handler)); }

		template<std::derived_from<CollisionController> Controller>
		void bind(const Collider& collider, void(Controller::* handler)(const ContactEventData&)) { bind(collider, static_cast<ContactHandler>(handler)); }
		template<std::derived_from<CollisionController> Controller>
		void bind(const Collider& collider, void(Controller::* handler)(const ContactEventData&) const) const { bind(collider, static_cast<ContactConstHandler>(handler)); }
		template<std::derived_from<CollisionController> Controller>
		void unbind(const Collider& collider, void(Controller::* handler)(const ContactEventData&)) { unbind(collider, static_cast<ContactHandler>(handler)); }
		template<std::derived_from<CollisionController> Controller>
		void unbind(const Collider& collider, void(Controller::* handler)(const ContactEventData&) const) const { unbind(collider, static_cast<ContactConstHandler>(handler)); }

		void emit(const Collider& from, OverlapHandler only_handler);
		void emit(const Collider& from, OverlapConstHandler only_handler) const;
		void emit(const Collider& from, CollisionHandler only_handler);
		void emit(const Collider& from, CollisionConstHandler only_handler) const;
		void emit(const Collider& from, ContactHandler only_handler);
		void emit(const Collider& from, ContactConstHandler only_handler) const;

		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const OverlapEventData&)) { emit(from, static_cast<OverlapHandler>(only_handler)); }
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const OverlapEventData&) const) const { emit(from, static_cast<OverlapConstHandler>(only_handler)); }
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const CollisionEventData&)) { emit(from, static_cast<CollisionHandler>(only_handler)); }
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const CollisionEventData&) const) const { emit(from, static_cast<CollisionConstHandler>(only_handler)); }
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const ContactEventData&)) { emit(from, static_cast<ContactHandler>(only_handler)); }
		template<std::derived_from<CollisionController> Controller>
		void emit(const Collider& from, void(Controller::* only_handler)(const ContactEventData&) const) const { emit(from, static_cast<ContactConstHandler>(only_handler)); }
	};
}
