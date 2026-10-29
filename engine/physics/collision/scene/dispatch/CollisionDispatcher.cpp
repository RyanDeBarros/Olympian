#include "CollisionDispatcher.h"

namespace oly::col2d
{
    // TODO this should just be for ostream&. Make Logger/Logger::Impl inherit from ostream.
    Logger::Impl operator<<(Logger::Impl log, Phase phase)
    {
        switch (phase)
        {
        case Phase::Started:
            return log << "started";
        case Phase::Ongoing:
            return log << "ongoing";
        case Phase::Completed:
            return log << "completed";
        default:
            return log << "expired";
        }
    }

    static Phase next_phase(bool overlap, Phase prior)
    {
        if (overlap)
            return prior & (Phase::Completed | Phase::Expired) ? Phase::Started : Phase::Ongoing;
        else
            return prior & (Phase::Started | Phase::Ongoing) ? Phase::Completed : Phase::Expired;
    }

    OverlapEventData::OverlapEventData(
        OverlapResult result,
        const Collider& active_collider,
        const Collider& passive_collider,
        Phase prior)
        : phase(next_phase(result.overlap, prior))
        , active_collider(active_collider)
        , passive_collider(passive_collider)
    {
    }

    OverlapEventData::OverlapEventData(Phase phase, const Collider& active_collider, const Collider& passive_collider)
        : phase(phase)
        , active_collider(active_collider)
        , passive_collider(passive_collider)
    {
    }

    OverlapEventData::OverlapEventData(const OverlapEventData& o)
        : phase(o.phase)
        , active_collider(o.active_collider)
        , passive_collider(o.passive_collider)
    {
    }

    OverlapEventData invert_event_data(const OverlapEventData& data)
    {
        return OverlapEventData(data.phase, data.passive_collider, data.active_collider);
    }

    CollisionEventData::CollisionEventData(
        const CollisionResult& result,
        const Collider& active_collider,
        const Collider& passive_collider,
        Phase prior)
        : phase(next_phase(result.overlap, prior))
        , active_collider(active_collider)
        , passive_collider(passive_collider)
        , penetration_depth(result.penetration_depth)
        , unit_impulse(result.unit_impulse)
    {
    }

    CollisionEventData::CollisionEventData(
        Phase phase,
        float penetration_depth,
        UnitVector2D unit_impulse,
        const Collider& active_collider,
        const Collider& passive_collider)
        : phase(phase)
        , penetration_depth(penetration_depth)
        , unit_impulse(unit_impulse)
        , active_collider(active_collider)
        , passive_collider(passive_collider)
    {
    }

    CollisionEventData::CollisionEventData(const CollisionEventData& o)
        : phase(o.phase)
        , penetration_depth(o.penetration_depth)
        , unit_impulse(o.unit_impulse)
        , active_collider(o.active_collider)
        , passive_collider(o.passive_collider)
    {
    }

    glm::vec2 CollisionEventData::mtv() const
    {
        return (glm::vec2)unit_impulse * penetration_depth;
    }

    OverlapEventData CollisionEventData::overlap_event() const
    {
        return OverlapEventData(phase, active_collider, passive_collider);
    }

    CollisionEventData invert_event_data(const CollisionEventData& data)
    {
        return CollisionEventData(
            data.phase,
            data.penetration_depth,
            -data.unit_impulse,
            data.passive_collider,
            data.active_collider);
    }

    ContactEventData::ContactEventData(
        const ContactResult& result,
        const Collider& active_collider,
        const Collider& passive_collider,
        Phase prior)
        : phase(next_phase(result.overlap, prior))
        , active_collider(active_collider)
        , passive_collider(passive_collider)
        , active_contact(result.active_contact)
        , passive_contact(result.passive_contact)
    {
    }

    ContactEventData::ContactEventData(
        Phase phase,
        ContactResult::Contact active_contact,
        ContactResult::Contact passive_contact,
        const Collider& active_collider,
        const Collider& passive_collider)
        : phase(phase)
        , active_contact(active_contact)
        , passive_contact(passive_contact)
        , active_collider(active_collider)
        , passive_collider(passive_collider)
    {
    }

    ContactEventData::ContactEventData(const ContactEventData& o)
        : phase(o.phase)
        , active_contact(o.active_contact)
        , passive_contact(o.passive_contact)
        , active_collider(o.active_collider)
        , passive_collider(o.passive_collider)
    {
    }

    OverlapEventData ContactEventData::overlap_event() const
    {
        return OverlapEventData(phase, active_collider, passive_collider);
    }

    CollisionEventData ContactEventData::collision_event() const
    {
        return CollisionEventData(
            phase,
            glm::length(active_contact.impulse),
            UnitVector2D(active_contact.impulse),
            active_collider,
            passive_collider);
    }

    ContactEventData invert_event_data(const ContactEventData& data)
    {
        return ContactEventData(
            data.phase,
            data.passive_contact,
            data.active_contact,
            data.passive_collider,
            data.active_collider);
    }

    Phase internal::CollisionPhaseTracker::prior_phase(const Collider& c1, const Collider& c2)
    {
        return map_.get_or(&c1, &c2, Phase::Expired);
    }

    void internal::CollisionPhaseTracker::lazy_update_phase(const Collider& c1, const Collider& c2, Phase phase)
    {
        lazy_updates_[{ &c1, &c2 }] = phase;
    }

    void internal::CollisionPhaseTracker::flush()
    {
        for (const auto& [pair, phase] : lazy_updates_)
            map_.set(pair, phase);
        lazy_updates_.clear();
    }

    void internal::CollisionPhaseTracker::clear()
    {
        map_.clear();
        lazy_updates_.clear();
    }

    void internal::CollisionPhaseTracker::copy_all(const Collider& from, const Collider& to)
    {
        flush();
        map_.copy_all(&from, &to);
    }

    void internal::CollisionPhaseTracker::replace_all(const Collider& at, const Collider& with)
    {
        flush();
        map_.replace_all(&at, &with);
    }

    void internal::CollisionPhaseTracker::erase_all(const Collider& c)
    {
        flush();
        map_.erase_all(&c);
    }

    void internal::CollisionCache::update(const Collider& c1, const Collider& c2, const OverlapEventData& data)
    {
        overlaps_.set(&c1, &c2, data);
    }

    void internal::CollisionCache::update(const Collider& c1, const Collider& c2, const CollisionEventData& data)
    {
        collisions_.set(&c1, &c2, data);
    }

    void internal::CollisionCache::update(const Collider& c1, const Collider& c2, const ContactEventData& data)
    {
        contacts_.set(&c1, &c2, data);
    }

    void internal::CollisionCache::clear()
    {
        overlaps_.clear();
        collisions_.clear();
        contacts_.clear();
    }

    void internal::CollisionCache::copy_all(const Collider& from, const Collider& to)
    {
        overlaps_.copy_all(&from, &to);
        collisions_.copy_all(&from, &to);
        contacts_.copy_all(&from, &to);
    }

    void internal::CollisionCache::replace_all(const Collider& at, const Collider& with)
    {
        overlaps_.replace_all(&at, &with);
        collisions_.replace_all(&at, &with);
        contacts_.replace_all(&at, &with);
    }

    void internal::CollisionCache::erase_all(const Collider& c)
    {
        overlaps_.erase_all(&c);
        collisions_.erase_all(&c);
        contacts_.erase_all(&c);
    }

    template<typename Result, typename EventData, typename HandlerRef>
    static void dispatch(
        const Collider& c1,
        const Collider& c2,
        std::unordered_map<const Collider*, HandlerRef>& handlers,
        Result(Collider::* method)(const Collider&) const,
        internal::CollisionPhaseTracker& phase_tracker,
        internal::CollisionCache& cache)
    {
        auto it_1 = handlers.find(&c1);
        auto it_2 = handlers.find(&c2);

        if (it_1 == handlers.end() && it_2 == handlers.end())
            return;

        EventData* data = nullptr;
        if (const std::optional<EventData>& d = cache.get<EventData>(c1, c2))
            data = new EventData(*d);
        else
        {
            if (!c1.one_way_blocks(c2) || !c2.one_way_blocks(c1))
                data = new EventData(Result(), c1, c2, phase_tracker.prior_phase(c1, c2));
            else
                data = new EventData((c1.*method)(c2), c1, c2, phase_tracker.prior_phase(c1, c2));
            cache.update(c1, c2, *data);
        }

        EventData& data1 = *data;
        phase_tracker.lazy_update_phase(c1, c2, data1.phase);
        if (data1.phase != Phase::Expired)
        {
            if (it_1 != handlers.end())
            {
                for (const auto& handler : it_1->second)
                    handler->invoke(data1);
            }

            if (it_2 != handlers.end())
            {
                EventData data2 = invert_event_data(data1);
                for (const auto& handler : it_2->second)
                    handler->invoke(data2);
            }
        }

        delete data;
    }

    CollisionDispatcher::CollisionDispatcher()
        : ITickService(TickPhase::Collision, TerminatePhase::Logic)
    {
    }

    size_t CollisionDispatcher::add_tree(
        const math::Rect2D bounds,
        const glm::uvec2 degree,
        const size_t cell_capacity)
    {
        trees_.emplace_back(bounds, degree, cell_capacity);
        return trees_.size() - 1;
    }

    const CollisionTree& CollisionDispatcher::get_tree(size_t i) const
    {
        return trees_[i];
    }

    void CollisionDispatcher::remove_tree(size_t i)
    {
        trees_.erase(trees_.begin() + i);
    }

    void CollisionDispatcher::clear()
    {
        trees_.clear();

        overlap_handler_map_.clear();
        collision_handler_map_.clear();
        contact_handler_map_.clear();

        overlap_controller_lut_.clear();
        collision_controller_lut_.clear();
        contact_controller_lut_.clear();

        collision_cache_.clear();
        phase_tracker_.clear();
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

    void CollisionDispatcher::unregister_overlap_handlers(const Collider& collider)
    {
        unregister_handlers_impl(collider, overlap_handler_map_, overlap_controller_lut_);
    }

    void CollisionDispatcher::unregister_collision_handlers(const Collider& collider)
    {
        unregister_handlers_impl(collider, collision_handler_map_, collision_controller_lut_);
    }

    void CollisionDispatcher::unregister_contact_handlers(const Collider& collider)
    {
        unregister_handlers_impl(collider, contact_handler_map_, contact_controller_lut_);
    }

    void CollisionDispatcher::unregister_handlers(const Collider& collider)
    {
        unregister_overlap_handlers(collider);
        unregister_collision_handlers(collider);
        unregister_contact_handlers(collider);
    }

    void CollisionDispatcher::on_tick()
    {
        collision_cache_.clear();
        phase_tracker_.flush();
        for (const CollisionTree& tree : trees_)
        {
            tree.flush();
            auto it = tree.iterator();
            while (!it.done())
            {
                auto pair = it.next();

                dispatch<ContactResult, ContactEventData>(
                    *pair.first,
                    *pair.second,
                    contact_handler_map_,
                    &Collider::contacts,
                    phase_tracker_,
                    collision_cache_);

                dispatch<CollisionResult, CollisionEventData>(
                    *pair.first,
                    *pair.second,
                    collision_handler_map_,
                    &Collider::collides,
                    phase_tracker_,
                    collision_cache_);

                dispatch<OverlapResult, OverlapEventData>(
                    *pair.first,
                    *pair.second,
                    overlap_handler_map_,
                    &Collider::overlaps,
                    phase_tracker_,
                    collision_cache_);
            }
        }
    }

    void CollisionDispatcher::emit(const Collider& from)
    {
        for (const CollisionTree& tree : trees_)
        {
            auto it = tree.query(from);
            while (!it.done())
            {
                const Collider* c2 = it.next();

                dispatch<ContactResult, ContactEventData>(
                    from,
                    *c2,
                    contact_handler_map_,
                    &Collider::contacts,
                    phase_tracker_,
                    collision_cache_);

                dispatch<CollisionResult, CollisionEventData>(
                    from,
                    *c2,
                    collision_handler_map_,
                    &Collider::collides,
                    phase_tracker_,
                    collision_cache_);

                dispatch<OverlapResult, OverlapEventData>(
                    from,
                    *c2,
                    overlap_handler_map_,
                    &Collider::overlaps,
                    phase_tracker_,
                    collision_cache_);
            }
        }
    }
}
