#include "CollisionController.h"

#include "physics/collision/scene/CollisionDispatcher.h"
#include "core/context/Collision.h"

namespace oly::col2d
{
	CollisionController::CollisionController()
	{
		context::collision_dispatcher().overlap_controller_lut[this];
		context::collision_dispatcher().collision_controller_lut[this];
		context::collision_dispatcher().contact_controller_lut[this];
	}
	
	template<typename LUT, typename Map>
	static void remove_controller(const CollisionController& controller, LUT& lut, Map& map)
	{
		auto it = lut.find(&controller);
		if (it != lut.end())
		{
			auto& lut_set = lut.find(&controller)->second;
			for (const auto& [collider, ref] : lut_set)
			{
				auto& handlers = map.find(collider)->second;
				handlers.erase(ref);
				if (handlers.empty())
					map.erase(collider);
			}
			lut.erase(it);
		}
	}

	CollisionController::~CollisionController()
	{
		auto& dispatcher = context::collision_dispatcher();
		remove_controller(*this, dispatcher.overlap_controller_lut, dispatcher.overlap_handler_map);
		remove_controller(*this, dispatcher.collision_controller_lut, dispatcher.collision_handler_map);
		remove_controller(*this, dispatcher.contact_controller_lut, dispatcher.contact_handler_map);
	}

#define BIND_HANDLER(Type, type)\
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();\
		dispatcher.type##_handler_map[&collider].insert(std::make_unique<internal::CollisionDispatcher::Type##HandlerRef>(*this, handler));

#define UNBIND_HANDLER(Type, type)\
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();\
		auto it = dispatcher.type##_handler_map.find(&collider);\
		if (it != dispatcher.type##_handler_map.end())\
		{\
			auto inner_it = it->second.find(std::make_unique<internal::CollisionDispatcher::Type##HandlerRef>(*this, handler));\
			if (inner_it != it->second.end())\
			{\
				it->second.erase(inner_it);\
				if (it->second.empty())\
					dispatcher.type##_handler_map.erase(it);\
			}\
		}

	void CollisionController::bind(const Collider& collider, OverlapHandler handler)
	{
		BIND_HANDLER(Overlap, overlap);
	}

	void CollisionController::bind(const Collider& collider, OverlapConstHandler handler) const
	{
		BIND_HANDLER(OverlapConst, overlap);
	}

	void CollisionController::unbind(const Collider& collider, OverlapHandler handler)
	{
		UNBIND_HANDLER(Overlap, overlap);
	}

	void CollisionController::unbind(const Collider& collider, OverlapConstHandler handler) const
	{
		UNBIND_HANDLER(OverlapConst, overlap);
	}

	void CollisionController::bind(const Collider& collider, CollisionHandler handler)
	{
		BIND_HANDLER(Collision, collision);
	}

	void CollisionController::bind(const Collider& collider, CollisionConstHandler handler) const
	{
		BIND_HANDLER(CollisionConst, collision);
	}

	void CollisionController::unbind(const Collider& collider, CollisionHandler handler)
	{
		UNBIND_HANDLER(Collision, collision);
	}

	void CollisionController::unbind(const Collider& collider, CollisionConstHandler handler) const
	{
		UNBIND_HANDLER(CollisionConst, collision);
	}

	void CollisionController::bind(const Collider& collider, ContactHandler handler)
	{
		BIND_HANDLER(Contact, contact);
	}

	void CollisionController::bind(const Collider& collider, ContactConstHandler handler) const
	{
		BIND_HANDLER(ContactConst, contact);
	}

	void CollisionController::unbind(const Collider& collider, ContactHandler handler)
	{
		UNBIND_HANDLER(Contact, contact);
	}

	void CollisionController::unbind(const Collider& collider, ContactConstHandler handler) const
	{
		UNBIND_HANDLER(ContactConst, contact);
	}

#undef BIND_HANDLER
#undef UNBIND_HANDLER

	template<typename Result, typename EventData, typename Handler>
	static void emit_from(const std::vector<CollisionTree>& trees, const Collider& from, Handler only_handler, CollisionController& only_controller,
		Result(Collider::* method)(const Collider&) const, internal::CollisionPhaseTracker& phase_tracker)
	{
		for (const CollisionTree& tree : trees)
		{
			auto it = tree.query(from);
			while (!it.done())
			{
				const Collider* other = it.next();
				EventData data((from.*method)(*other), from, *other, phase_tracker.prior_phase(from, *other));
				phase_tracker.lazy_update_phase(from, *other, data.phase);
				if (data.phase != Phase::EXPIRED)
					(only_controller.*only_handler)(data);
			}
		}
	}

	void CollisionController::emit(const Collider& from, OverlapHandler only_handler)
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		emit_from<OverlapResult, OverlapEventData>(dispatcher.trees, from, only_handler, *this, &Collider::overlaps, dispatcher.phase_tracker);
	}

	void CollisionController::emit(const Collider& from, OverlapConstHandler only_handler) const
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		emit_from<OverlapResult, OverlapEventData>(dispatcher.trees, from, only_handler, const_cast<CollisionController&>(*this), &Collider::overlaps, dispatcher.phase_tracker);
	}

	void CollisionController::emit(const Collider& from, CollisionHandler only_handler)
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		emit_from<CollisionResult, CollisionEventData>(dispatcher.trees, from, only_handler, *this, &Collider::collides, dispatcher.phase_tracker);
	}

	void CollisionController::emit(const Collider& from, CollisionConstHandler only_handler) const
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		emit_from<CollisionResult, CollisionEventData>(dispatcher.trees, from, only_handler, const_cast<CollisionController&>(*this), &Collider::collides, dispatcher.phase_tracker);
	}

	void CollisionController::emit(const Collider& from, ContactHandler only_handler)
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		emit_from<ContactResult, ContactEventData>(dispatcher.trees, from, only_handler, *this, &Collider::contacts, dispatcher.phase_tracker);
	}

	void CollisionController::emit(const Collider& from, ContactConstHandler only_handler) const
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		emit_from<ContactResult, ContactEventData>(dispatcher.trees, from, only_handler, const_cast<CollisionController&>(*this), &Collider::contacts, dispatcher.phase_tracker);
	}

}
