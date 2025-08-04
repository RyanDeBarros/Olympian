#include "CollisionDispatcher.h"

namespace oly::col2d
{
	Logger& operator<<(Logger& log, Phase phase)
	{
#pragma warning(suppress : 26813)
		return log << (phase == Phase::STARTED ? "started" : phase == Phase::ONGOING ? "ongoing" : phase == Phase::COMPLETED ? "completed" : "expired");
	}

	static Phase next_phase(bool overlap, Phase prior)
	{
		if (overlap)
			return prior & (Phase::COMPLETED | Phase::EXPIRED) ? Phase::STARTED : Phase::ONGOING;
		else
			return prior & (Phase::STARTED | Phase::ONGOING) ? Phase::COMPLETED : Phase::EXPIRED;
	}

	OverlapEventData::OverlapEventData(OverlapResult result, const ConstSoftReference<Collider>& active_collider, const ConstSoftReference<Collider>& passive_collider, Phase prior)
		: phase(next_phase(result.overlap, prior)), active_collider(active_collider), passive_collider(passive_collider)
	{
	}

	CollisionEventData::CollisionEventData(const CollisionResult& result, const ConstSoftReference<Collider>& active_collider, const ConstSoftReference<Collider>& passive_collider, Phase prior)
		: phase(next_phase(result.overlap, prior)), active_collider(active_collider), passive_collider(passive_collider),
		penetration_depth(result.penetration_depth), unit_impulse(result.unit_impulse)
	{
	}

	ContactEventData::ContactEventData(const ContactResult& result, const ConstSoftReference<Collider>& active_collider, const ConstSoftReference<Collider>& passive_collider, Phase prior)
		: phase(next_phase(result.overlap, prior)), active_collider(active_collider), passive_collider(passive_collider),
		active_contact(result.active_contact), passive_contact(result.passive_contact)
	{
	}

	Phase CollisionPhaseTracker::prior_phase(const ConstSoftReference<Collider>& c1, const ConstSoftReference<Collider>& c2)
	{
		auto it = map.find({ c1, c2 });
		if (it != map.end())
			return it->second;
		else
		{
			map[{ c1, c2 }] = Phase::EXPIRED;
			return Phase::EXPIRED;
		}
	}
	
	void CollisionPhaseTracker::lazy_update_phase(const ConstSoftReference<Collider>& c1, const ConstSoftReference<Collider>& c2, Phase phase)
	{
		lazy_updates[{ c1, c2 }] = phase;
	}
	
	void CollisionPhaseTracker::flush()
	{
		for (const auto& [pair, phase] : lazy_updates)
			if (pair.c1 && pair.c2)
				map[pair] = phase;
		lazy_updates.clear();
	}
	
	void CollisionPhaseTracker::clear()
	{
		map.clear();
	}

	void CollisionPhaseTracker::clean()
	{
		for (auto it = map.begin(); it != map.end(); )
		{
			if (!it->first.c1 || !it->first.c2)
				it = map.erase(it);
			else
				++it;
		}
	}

	void CollisionPhaseTracker::erase(const ConstSoftReference<Collider>& c1, const ConstSoftReference<Collider>& c2)
	{
		map.erase({ c1, c2 });
	}

	template<typename Result, typename EventData, typename HandlerRef>
	static void dispatch(const ConstSoftReference<Collider>& first, const ConstSoftReference<Collider>& second,
		std::unordered_map<ConstSoftReference<Collider>, HandlerRef>& handlers, Result(Collider::*method)(const Collider&) const, CollisionPhaseTracker& phase_tracker)
	{
		static const auto handler_loop = [](const auto& outer_it, const auto& data)
			{
				for (auto it = outer_it->second.begin(); it != outer_it->second.end(); )
				{
					if ((*it)->controller)
					{
						(*it)->invoke(data);
						++it;
					}
					else
						it = outer_it->second.erase(it);
				}
			};

		auto it_1 = handlers.find(first);
		auto it_2 = handlers.find(second);

		if (it_1 == handlers.end() && it_2 == handlers.end())
			return;

		const Collider* c1 = first.get();
		const Collider* c2 = second.get();
		if (!c1)
		{
			handlers.erase(it_1);
			if (!c2)
			{
				it_2 = handlers.find(second);
				handlers.erase(it_2);
			}
			return;
		}
		else if (!c2)
		{
			handlers.erase(it_2);
			return;
		}

		EventData data((c1->*method)(*c2), first, second, phase_tracker.prior_phase(first, second));
		phase_tracker.lazy_update_phase(first, second, data.phase);
		if (data.phase == Phase::EXPIRED)
			return;

		if (it_1 != handlers.end())
			handler_loop(it_1, data);
		if (it_2 != handlers.end())
			handler_loop(it_2, data.invert());

		if (it_1 != handlers.end() && it_1->second.empty())
		{
			handlers.erase(it_1);
			it_2 = handlers.find(second);
		}
		if (it_2 != handlers.end() && it_2->second.empty())
			handlers.erase(it_2);
	}

	size_t internal::CollisionDispatcher::add_tree(const math::Rect2D bounds, const glm::uvec2 degree, const size_t cell_capacity)
	{
		trees.emplace_back(bounds, degree, cell_capacity);
		return trees.size() - 1;
	}

	void internal::CollisionDispatcher::clear()
	{
		trees.clear();
		overlap_handlers.clear();
		collision_handlers.clear();
		contact_handlers.clear();
		phase_tracker.clear();
	}

	void internal::CollisionDispatcher::poll() const
	{
		phase_tracker.flush();
		for (const CollisionTree& tree : trees)
		{
			tree.flush();
			auto it = tree.iterator();
			while (!it.done())
			{
				auto pair = it.next();
				dispatch<OverlapResult, OverlapEventData>(pair.first, pair.second, overlap_handlers, &Collider::overlaps, phase_tracker);
				dispatch<CollisionResult, CollisionEventData>(pair.first, pair.second, collision_handlers, &Collider::collides, phase_tracker);
				dispatch<ContactResult, ContactEventData>(pair.first, pair.second, contact_handlers, &Collider::contacts, phase_tracker);
			}
		}
	}

	template<typename HandlerRef>
	static void clean_handlers(std::unordered_map<ConstSoftReference<Collider>, HandlerRef>& handlers)
	{
		for (auto it = handlers.begin(); it != handlers.end(); )
		{
			if (it->first)
			{
				for (auto inner_it = it->second.begin(); inner_it != it->second.end(); )
				{
					if ((*inner_it)->controller)
						++inner_it;
					else
						inner_it = it->second.erase(inner_it);
				}
				if (it->second.empty())
					it = handlers.erase(it);
				else
					++it;
			}
			else
				it = handlers.erase(it);
		}
	}

	void internal::CollisionDispatcher::clean()
	{
		clean_handlers(overlap_handlers);
		clean_handlers(collision_handlers);
		clean_handlers(contact_handlers);
	}

	void internal::CollisionDispatcher::emit(const Collider& from)
	{
		ConstSoftReference<Collider> c1 = from.cref();
		for (const CollisionTree& tree : trees)
		{
			auto it = tree.query(from);
			while (!it.done())
			{
				ConstSoftReference<Collider> c2 = it.next();
				dispatch<OverlapResult, OverlapEventData>(c1, c2, overlap_handlers, &Collider::overlaps, phase_tracker);
				dispatch<CollisionResult, CollisionEventData>(c1, c2, collision_handlers, &Collider::collides, phase_tracker);
				dispatch<ContactResult, ContactEventData>(c1, c2, contact_handlers, &Collider::contacts, phase_tracker);
			}
		}
	}
}
