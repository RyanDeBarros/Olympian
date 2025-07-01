#include "CollisionDispatcher.h"

namespace oly::col2d
{
	template<typename EventData, typename HandlerRef, typename Method>
	static void dispatch(const CollisionTree::PairIterator::ColliderPtrPair& pair, std::unordered_map<ConstSoftReference<Collider>, HandlerRef>& handlers, Method method)
	{
		auto it_1 = handlers.find(pair.first);
		bool found_1 = it_1 != handlers.end();
		if (found_1 && !it_1->second.controller)
		{
			found_1 = false;
			handlers.erase(it_1);
		}

		auto it_2 = handlers.find(pair.second);
		bool found_2 = it_2 != handlers.end();
		if (found_2 && !it_2->second.controller)
		{
			found_2 = false;
			handlers.erase(it_2);
		}

		if (found_1 || found_2)
		{
			// pair.first and pair.second are guaranteed to be valid, since tree was flushed prior.
			EventData data = { .result = (pair.first.get()->*method)(*pair.second), .active_collider = pair.first, .passive_collider = pair.second };
			if (found_1)
				(it_1->second.controller.get()->*it_1->second.handler)(data);
			if (found_2)
				(it_2->second.controller.get()->*it_2->second.handler)(data.invert());
		}
	}

	void CollisionDispatcher::poll()
	{
		tree.flush();
		auto it = tree.iterator();
		while (!it.done())
		{
			auto pair = it.next();
			dispatch<OverlapEventData>(pair, overlap_handlers, &Collider::overlaps);
			dispatch<CollisionEventData>(pair, collision_handlers, &Collider::collides);
			dispatch<ContactEventData>(pair, contact_handlers, &Collider::contacts);
		}
	}

	template<typename HandlerRef>
	static void clean_handlers(std::unordered_map<ConstSoftReference<Collider>, HandlerRef>& handlers)
	{
		for (auto it = handlers.begin(); it != handlers.end(); )
		{
			if (it->first && it->second.controller)
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
}
