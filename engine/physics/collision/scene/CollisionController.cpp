#include "CollisionController.h"

#include "physics/collision/scene/CollisionDispatcher.h"
#include "core/context/Collision.h"

namespace oly::col2d
{
	void CollisionController::bind(const ConstSoftReference<Collider>& collider, OverlapHandler handler)
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		dispatcher.overlap_handlers[collider].insert(std::make_unique<internal::CollisionDispatcher::OverlapHandlerRef>(ref(), handler));
	}

	void CollisionController::bind(const ConstSoftReference<Collider>& collider, OverlapConstHandler handler) const
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		dispatcher.overlap_handlers[collider].insert(std::make_unique<internal::CollisionDispatcher::OverlapConstHandlerRef>(cref(), handler));
	}

	void CollisionController::unbind(const ConstSoftReference<Collider>& collider, OverlapHandler handler)
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		auto it = dispatcher.overlap_handlers.find(collider);
		if (it != dispatcher.overlap_handlers.end())
		{
			auto inner_it = it->second.find(std::make_unique<internal::CollisionDispatcher::OverlapHandlerRef>(ref(), handler));
			if (inner_it != it->second.end())
			{
				it->second.erase(inner_it);
				if (it->second.empty())
					dispatcher.overlap_handlers.erase(it);
			}
		}
	}

	void CollisionController::unbind(const ConstSoftReference<Collider>& collider, OverlapConstHandler handler) const
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		auto it = dispatcher.overlap_handlers.find(collider);
		if (it != dispatcher.overlap_handlers.end())
		{
			auto inner_it = it->second.find(std::make_unique<internal::CollisionDispatcher::OverlapConstHandlerRef>(cref(), handler));
			if (inner_it != it->second.end())
			{
				it->second.erase(inner_it);
				if (it->second.empty())
					dispatcher.overlap_handlers.erase(it);
			}
		}
	}

	void CollisionController::bind(const ConstSoftReference<Collider>& collider, CollisionHandler handler)
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		dispatcher.collision_handlers[collider].insert(std::make_unique<internal::CollisionDispatcher::CollisionHandlerRef>(ref(), handler));
	}

	void CollisionController::bind(const ConstSoftReference<Collider>& collider, CollisionConstHandler handler) const
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		dispatcher.collision_handlers[collider].insert(std::make_unique<internal::CollisionDispatcher::CollisionConstHandlerRef>(cref(), handler));
	}

	void CollisionController::unbind(const ConstSoftReference<Collider>& collider, CollisionHandler handler)
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		auto it = dispatcher.collision_handlers.find(collider);
		if (it != dispatcher.collision_handlers.end())
		{
			auto inner_it = it->second.find(std::make_unique<internal::CollisionDispatcher::CollisionHandlerRef>(ref(), handler));
			if (inner_it != it->second.end())
			{
				it->second.erase(inner_it);
				if (it->second.empty())
					dispatcher.collision_handlers.erase(it);
			}
		}
	}

	void CollisionController::unbind(const ConstSoftReference<Collider>& collider, CollisionConstHandler handler) const
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		auto it = dispatcher.collision_handlers.find(collider);
		if (it != dispatcher.collision_handlers.end())
		{
			auto inner_it = it->second.find(std::make_unique<internal::CollisionDispatcher::CollisionConstHandlerRef>(cref(), handler));
			if (inner_it != it->second.end())
			{
				it->second.erase(inner_it);
				if (it->second.empty())
					dispatcher.collision_handlers.erase(it);
			}
		}
	}

	void CollisionController::bind(const ConstSoftReference<Collider>& collider, ContactHandler handler)
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		dispatcher.contact_handlers[collider].insert(std::make_unique<internal::CollisionDispatcher::ContactHandlerRef>(ref(), handler));
	}

	void CollisionController::bind(const ConstSoftReference<Collider>& collider, ContactConstHandler handler) const
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		dispatcher.contact_handlers[collider].insert(std::make_unique<internal::CollisionDispatcher::ContactConstHandlerRef>(cref(), handler));
	}

	void CollisionController::unbind(const ConstSoftReference<Collider>& collider, ContactHandler handler)
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		auto it = dispatcher.contact_handlers.find(collider);
		if (it != dispatcher.contact_handlers.end())
		{
			auto inner_it = it->second.find(std::make_unique<internal::CollisionDispatcher::ContactHandlerRef>(ref(), handler));
			if (inner_it != it->second.end())
			{
				it->second.erase(inner_it);
				if (it->second.empty())
					dispatcher.contact_handlers.erase(it);
			}
		}
	}

	void CollisionController::unbind(const ConstSoftReference<Collider>& collider, ContactConstHandler handler) const
	{
		internal::CollisionDispatcher& dispatcher = context::collision_dispatcher();
		auto it = dispatcher.contact_handlers.find(collider);
		if (it != dispatcher.contact_handlers.end())
		{
			auto inner_it = it->second.find(std::make_unique<internal::CollisionDispatcher::ContactConstHandlerRef>(cref(), handler));
			if (inner_it != it->second.end())
			{
				it->second.erase(inner_it);
				if (it->second.empty())
					dispatcher.contact_handlers.erase(it);
			}
		}
	}

	template<typename Result, typename EventData, typename Handler>
	static void emit_from(const std::vector<CollisionTree>& trees, const Collider& from, Handler only_handler, CollisionController& only_controller,
		Result(Collider::* method)(const Collider&) const, CollisionPhaseTracker& phase_tracker)
	{
		ConstSoftReference<Collider> c1 = from.cref();
		for (const CollisionTree& tree : trees)
		{
			auto it = tree.query(from);
			while (!it.done())
			{
				ConstSoftReference<Collider> other = it.next();
				if (const Collider* c2 = other.get())
				{
					EventData data((from.*method)(*c2), c1, other, phase_tracker.prior_phase(c1, other));
					phase_tracker.lazy_update_phase(c1, other, data.phase);
					if (data.phase != Phase::EXPIRED)
						(only_controller.*only_handler)(data);
				}
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
