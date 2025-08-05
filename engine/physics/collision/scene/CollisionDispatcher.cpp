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

	OverlapEventData::OverlapEventData(OverlapResult result, const Collider& active_collider, const Collider& passive_collider, Phase prior)
		: phase(next_phase(result.overlap, prior)), active_collider(active_collider), passive_collider(passive_collider)
	{
	}

	CollisionEventData::CollisionEventData(const CollisionResult& result, const Collider& active_collider, const Collider& passive_collider, Phase prior)
		: phase(next_phase(result.overlap, prior)), active_collider(active_collider), passive_collider(passive_collider),
		penetration_depth(result.penetration_depth), unit_impulse(result.unit_impulse)
	{
	}

	ContactEventData::ContactEventData(const ContactResult& result, const Collider& active_collider, const Collider& passive_collider, Phase prior)
		: phase(next_phase(result.overlap, prior)), active_collider(active_collider), passive_collider(passive_collider),
		active_contact(result.active_contact), passive_contact(result.passive_contact)
	{
	}

	Phase internal::CollisionPhaseTracker::prior_phase(const Collider& c1, const Collider& c2)
	{
		auto it = map.find({ &c1, &c2 });
		if (it != map.end())
			return it->second;
		else
		{
			map[{ &c1, &c2 }] = Phase::EXPIRED;
			lut[&c1].insert(&c2);
			lut[&c2].insert(&c1);
			return Phase::EXPIRED;
		}
	}
	
	void internal::CollisionPhaseTracker::lazy_update_phase(const Collider& c1, const Collider& c2, Phase phase)
	{
		lazy_updates[{ &c1, &c2 }] = phase;
	}
	
	void internal::CollisionPhaseTracker::flush()
	{
		for (const auto& [pair, phase] : lazy_updates)
			if (pair.c1 && pair.c2)
				map[pair] = phase;
		lazy_updates.clear();
	}
	
	void internal::CollisionPhaseTracker::clear()
	{
		map.clear();
		lazy_updates.clear();
		lut.clear();
	}

	void internal::CollisionPhaseTracker::copy_all(const Collider& from, const Collider& to)
	{
		flush();

		auto it = lut.find(&from);
		if (it != lut.end())
		{
			auto& lut_og = it->second;
			auto& lut_copy = lut[&to];
			for (const Collider* c : lut_og)
			{
				lut_copy.insert(c);
				map[{ &to, c }] = map.find({ &from, c })->second;
			}
		}
	}

	void internal::CollisionPhaseTracker::replace_all(const Collider& at, const Collider& with)
	{
		flush();

		auto it = lut.find(&with);
		if (it != lut.end())
		{
			auto& lut_og = it->second;
			auto& lut_copy = lut[&at];
			for (const Collider* c : lut_og)
			{
				lut_copy.insert(c);
				map[{ &at, c }] = map.find({ &with, c })->second;
				map.erase({ &with, c });
			}
			lut.erase(&with);
		}
	}

	void internal::CollisionPhaseTracker::erase_all(const Collider& c)
	{
		flush();

		auto it = lut.find(&c);
		if (it != lut.end())
		{
			for (const Collider* col : it->second)
				map.erase({ &c, col });
			lut.erase(it);
		}
	}

	template<typename Result, typename EventData, typename HandlerRef>
	static void dispatch(const Collider& c1, const Collider& c2, std::unordered_map<const Collider*, HandlerRef>& handlers,
		Result(Collider::*method)(const Collider&) const, internal::CollisionPhaseTracker& phase_tracker)
	{
		auto it_1 = handlers.find(&c1);
		auto it_2 = handlers.find(&c2);

		if (it_1 == handlers.end() && it_2 == handlers.end())
			return;

		EventData data1((c1.*method)(c2), c1, c2, phase_tracker.prior_phase(c1, c2));
		phase_tracker.lazy_update_phase(c1, c2, data1.phase);
		if (data1.phase != Phase::EXPIRED)
		{
			if (it_1 != handlers.end())
				for (const auto& handler : it_1->second)
					handler->invoke(data1);
		
			if (it_2 != handlers.end())
			{
				EventData data2 = invert_event_data(data1);
				for (const auto& handler : it_2->second)
					handler->invoke(data2);
			}
		}
	}

	size_t internal::CollisionDispatcher::add_tree(const math::Rect2D bounds, const glm::uvec2 degree, const size_t cell_capacity)
	{
		trees.emplace_back(bounds, degree, cell_capacity);
		return trees.size() - 1;
	}

	void internal::CollisionDispatcher::clear()
	{
		trees.clear();
		
		overlap_handler_map.clear();
		collision_handler_map.clear();
		contact_handler_map.clear();
		
		overlap_controller_lut.clear();
		collision_controller_lut.clear();
		contact_controller_lut.clear();
		
		phase_tracker.clear();
	}

	template<typename Map, typename LUT>
	static void unregister_handlers_impl(const Collider& collider, Map& map, LUT& lut)
	{
		auto it = map.find(&collider);
		if (it != map.end())
		{
			auto& handlers = it->second;
			for (auto& ref : handlers)
			{
				auto& lut_set = lut.find(ref->controller)->second;
				lut_set.erase(std::make_pair(&collider, ref->clone()));
			}
			map.erase(it);
		}
	}

	void internal::CollisionDispatcher::unregister_overlap_handlers(const Collider& collider)
	{
		unregister_handlers_impl(collider, overlap_handler_map, overlap_controller_lut);
	}

	void internal::CollisionDispatcher::unregister_collision_handlers(const Collider& collider)
	{
		unregister_handlers_impl(collider, collision_handler_map, collision_controller_lut);
	}

	void internal::CollisionDispatcher::unregister_contact_handlers(const Collider& collider)
	{
		unregister_handlers_impl(collider, contact_handler_map, contact_controller_lut);
	}

	void internal::CollisionDispatcher::unregister_handlers(const Collider& collider)
	{
		unregister_overlap_handlers(collider);
		unregister_collision_handlers(collider);
		unregister_contact_handlers(collider);
	}

	void internal::CollisionDispatcher::poll()
	{
		phase_tracker.flush();
		for (const CollisionTree& tree : trees)
		{
			tree.flush();
			auto it = tree.iterator();
			while (!it.done())
			{
				auto pair = it.next();
				dispatch<OverlapResult, OverlapEventData>(*pair.first, *pair.second, overlap_handler_map, &Collider::overlaps, phase_tracker);
				dispatch<CollisionResult, CollisionEventData>(*pair.first, *pair.second, collision_handler_map, &Collider::collides, phase_tracker);
				dispatch<ContactResult, ContactEventData>(*pair.first, *pair.second, contact_handler_map, &Collider::contacts, phase_tracker);
			}
		}
	}

	void internal::CollisionDispatcher::emit(const Collider& from)
	{
		for (const CollisionTree& tree : trees)
		{
			auto it = tree.query(from);
			while (!it.done())
			{
				const Collider* c2 = it.next();
				dispatch<OverlapResult, OverlapEventData>(from, *c2, overlap_handler_map, &Collider::overlaps, phase_tracker);
				dispatch<CollisionResult, CollisionEventData>(from, *c2, collision_handler_map, &Collider::collides, phase_tracker);
				dispatch<ContactResult, ContactEventData>(from, *c2, contact_handler_map, &Collider::contacts, phase_tracker);
			}
		}
	}
}
