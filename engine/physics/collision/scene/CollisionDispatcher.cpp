#include "CollisionDispatcher.h"

namespace oly::col2d
{
	template<typename Result, typename Method>
	static void dispatch(const CollisionTree::PairIterator::ColliderPtrPair& pair, const std::unordered_map<ConstSoftReference<Collider>, std::unique_ptr<EventHandler<Result>>>& handlers, Method method)
	{
		auto it_1 = handlers.find(pair.first);
		auto it_2 = handlers.find(pair.second);

		if (it_1 != handlers.end() || it_2 != handlers.end())
		{
			Result result = (pair.first.get()->*method)(*pair.second);
			if (it_1 != handlers.end())
				it_1->second->handle(result);
			if (it_2 != handlers.end())
				it_2->second->handle(result.invert());
		}
	}

	void CollisionDispatcher::poll()
	{
		tree.flush();
		auto it = tree.iterator();
		while (!it.done())
		{
			auto pair = it.next();
			dispatch(pair, overlap_handlers, &Collider::overlaps);
			dispatch(pair, collision_handlers, &Collider::collides);
			dispatch(pair, contact_handlers, &Collider::contacts);
		}
	}
}
