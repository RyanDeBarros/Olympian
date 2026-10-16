#pragma once

#include "core/base/Assert.h"

#include "graphics/backend/specialized/LightweightBuffers.h"

#include <imp/id_generator.hpp>

#include <unordered_map>

namespace oly::graphics
{
    template<
        typename StoredObjectType,
        std::integral SlotType,
        typename StoredObjectTypeHash = std::hash<StoredObjectType>>
    class UsageSlotTracker
    {
        struct UsageHolder
        {
            StoredObjectType obj = {};
            GLuint usage = 0;
        };

        std::unordered_map<SlotType, UsageHolder> usages_;
        std::unordered_map<StoredObjectType, SlotType, StoredObjectTypeHash> slot_lookup_;
        imp::soft_id_generator<SlotType> pos_generator_;

    public:
        UsageSlotTracker()
        {
            pos_generator_.gen();
            // waste 0th slot
        }

        std::optional<StoredObjectType> decrement_usage(SlotType i)
        {
            if (i != 0)
                return decrement_usage_impl(i);
            else
                return std::nullopt;
        }

        bool set_object(
            LightweightBuffer<Mutability::Mutable>& buffer,
            SlotType& slot,
            const StoredObjectType& stored_obj)
        {
            std::optional<StoredObjectType> prev_obj;
            return set_object<StoredObjectType>(buffer, slot, stored_obj, stored_obj, prev_obj);
        }

        template<typename BufferObjectType>
        bool set_object(
            LightweightBuffer<Mutability::Mutable>& buffer,
            SlotType& slot,
            const StoredObjectType& stored_obj,
            const BufferObjectType& buffer_obj,
            std::optional<StoredObjectType>& prev_obj)
        {
            if (stored_obj == StoredObjectType{}) // remove object from batch
            {
                if (slot != 0)
                {
                    prev_obj = decrement_usage_impl(slot);
                    slot = 0;
                    return true; // slot has changed
                }

                return false; // slot did not change
            }

            if (slot != 0) // slot references existing object -> decrement its usage
            {
                auto it = usages_.find(slot);
                if (stored_obj == it->second.obj) // same object that exists -> do nothing
                    return false; // slot did not change

                --it->second.usage;
                if (it->second.usage == 0)
                    prev_obj = erase_slot(it);
            }
            
            auto newit = slot_lookup_.find(stored_obj);
            if (newit != slot_lookup_.end()) // object already exists -> increment its usage
            {
                ++usages_.find(newit->second)->second.usage;
                slot = newit->second;
            }
            else // create new object slot
            {
                slot = pos_generator_.gen();
                usages_[slot] = { stored_obj, 1 };
                slot_lookup_[stored_obj] = slot;
                OLY_ASSERT(slot * sizeof(BufferObjectType) <= (GLuint)buffer.get_size());
                if (slot * sizeof(BufferObjectType) == buffer.get_size())
                    buffer.resize(buffer.get_size() + sizeof(BufferObjectType));
                buffer.send<BufferObjectType>(slot, buffer_obj);
            }

            return true; // slot has changed
        }

        const StoredObjectType& get_object(SlotType slot) const
        {
            return usages_.find(slot)->second.obj;
        }

        StoredObjectType& get_object(SlotType slot)
        {
            return usages_.find(slot)->second.obj;
        }

        bool get_slot(const StoredObjectType& obj, SlotType& slot) const
        {
            auto it = slot_lookup_.find(obj);
            if (it != slot_lookup_.end())
            {
                slot = it->second;
                return true;
            }

            return false;
        }

    private:
        std::optional<StoredObjectType> decrement_usage_impl(SlotType i)
        {
            auto it = usages_.find(i);
            --it->second.usage;
            if (it->second.usage == 0)
                return erase_slot(it);
            else
                return std::nullopt;
        }

        StoredObjectType erase_slot(typename decltype(usages_)::iterator& it)
        {
            pos_generator_.yield(it->first);
            StoredObjectType obj = std::move(it->second.obj);
            slot_lookup_.erase(obj);
            usages_.erase(it);
            return obj;
        }
    };
}
