#include "CollisionDispatcher.h"

namespace oly::col2d
{
	template<typename Result, typename EventData, typename HandlerRef>
	static void dispatch(const ConstSoftReference<Collider>& first, const ConstSoftReference<Collider>& second, std::unordered_map<ConstSoftReference<Collider>,
		HandlerRef>& handlers, Result(Collider::*method)(const Collider&) const)
	{
		static const auto invalid_controller = [](auto&& ref) { return !ref.controller; };
		static const auto emit = [](auto&& ref, const auto& data) { return (ref.controller.get()->*ref.handler)(data); };

		auto it_1 = handlers.find(first);
		bool found_1 = it_1 != handlers.end();
		if (found_1 && std::visit(invalid_controller, it_1->second))
		{
			found_1 = false;
			handlers.erase(it_1);
		}

		auto it_2 = handlers.find(second);
		bool found_2 = it_2 != handlers.end();
		if (found_2 && std::visit(invalid_controller, it_2->second))
		{
			found_2 = false;
			handlers.erase(it_2);
		}

		if (found_1 || found_2)
		{
			const Collider* c1 = first.get();
			const Collider* c2 = second.get();
			if (c1 && c2)
			{
				EventData data = { .result = (c1->*method)(*c2), .active_collider = first, .passive_collider = second };
				if (found_1)
					std::visit([&data](auto&& ref) { emit(ref, data); }, it_1->second);
				if (found_2)
					std::visit([&data](auto&& ref) { emit(ref, data.invert()); }, it_2->second);
			}
		}
	}

	void CollisionDispatcher::poll()
	{
		tree.flush();
		auto it = tree.iterator();
		while (!it.done())
		{
			auto pair = it.next();
			dispatch<OverlapResult, OverlapEventData>(pair.first, pair.second, overlap_handlers, &Collider::overlaps);
			dispatch<CollisionResult, CollisionEventData>(pair.first, pair.second, collision_handlers, &Collider::collides);
			dispatch<ContactResult, ContactEventData>(pair.first, pair.second, contact_handlers, &Collider::contacts);
		}
	}

	template<typename HandlerRef>
	static void clean_handlers(std::unordered_map<ConstSoftReference<Collider>, HandlerRef>& handlers)
	{
		static const auto valid_controller = [](auto&& ref) -> bool { return ref.controller; };

		for (auto it = handlers.begin(); it != handlers.end(); )
		{
			if (it->first && std::visit(valid_controller, it->second))
				++it;
			else
				it = handlers.erase(it);
		}
	}

	void CollisionDispatcher::clean()
	{
		clean_handlers(overlap_handlers);
		clean_handlers(collision_handlers);
		clean_handlers(contact_handlers);
	}

	void CollisionDispatcher::emit(const Collider& from)
	{
		ConstSoftReference<Collider> c1 = from.cref();
		auto it = tree.query(from);
		while (!it.done())
		{
			ConstSoftReference<Collider> c2 = it.next();
			dispatch<OverlapResult, OverlapEventData>(c1, c2, overlap_handlers, &Collider::overlaps);
			dispatch<CollisionResult, CollisionEventData>(c1, c2, collision_handlers, &Collider::collides);
			dispatch<ContactResult, ContactEventData>(c1, c2, contact_handlers, &Collider::contacts);
		}
	}

	template<typename Result, typename EventData, typename Handler, typename Reference>
	static void emit_from(const CollisionTree& tree, const Collider& from, Handler only_handler, const Reference& only_controller,
		Result(Collider::*method)(const Collider&) const)
	{
		ConstSoftReference<Collider> c1 = from.cref();
		auto it = tree.query(from);
		while (!it.done())
		{
			ConstSoftReference<Collider> other = it.next();
			if (const Collider* c2 = other.get())
			{
				EventData data = { .result = (from.*method)(*c2), .active_collider = c1, .passive_collider = other};
				(only_controller.get()->*only_handler)(data);
			}
		}
	}
	
	void CollisionDispatcher::emit(const Collider& from, CollisionController::OverlapHandler only_handler, const SoftReference<CollisionController>& only_controller) const
	{
		emit_from<OverlapResult, OverlapEventData>(tree, from, only_handler, only_controller, &Collider::overlaps);
	}

	void CollisionDispatcher::emit(const Collider& from, CollisionController::OverlapConstHandler only_handler, const ConstSoftReference<CollisionController>& only_controller) const
	{
		emit_from<OverlapResult, OverlapEventData>(tree, from, only_handler, only_controller, &Collider::overlaps);
	}
	
	void CollisionDispatcher::emit(const Collider& from, CollisionController::CollisionHandler only_handler, const SoftReference<CollisionController>& only_controller) const
	{
		emit_from<CollisionResult, CollisionEventData>(tree, from, only_handler, only_controller, &Collider::collides);
	}

	void CollisionDispatcher::emit(const Collider& from, CollisionController::CollisionConstHandler only_handler, const ConstSoftReference<CollisionController>& only_controller) const
	{
		emit_from<CollisionResult, CollisionEventData>(tree, from, only_handler, only_controller, &Collider::collides);
	}
	
	void CollisionDispatcher::emit(const Collider& from, CollisionController::ContactHandler only_handler, const SoftReference<CollisionController>& only_controller) const
	{
		emit_from<ContactResult, ContactEventData>(tree, from, only_handler, only_controller, &Collider::contacts);
	}

	void CollisionDispatcher::emit(const Collider& from, CollisionController::ContactConstHandler only_handler, const ConstSoftReference<CollisionController>& only_controller) const
	{
		emit_from<ContactResult, ContactEventData>(tree, from, only_handler, only_controller, &Collider::contacts);
	}
}
