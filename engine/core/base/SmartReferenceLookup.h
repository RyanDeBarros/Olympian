#pragma once

#include "core/base/SmartReference.h"

#include <unordered_map>

namespace oly
{
    template<typename Object, typename Constructor>
    class SmartReferenceLookup
    {
        struct Hash
        {
            size_t operator()(Constructor ctor) const
            {
                return ctor.hash();
            }
        };

    private:
        std::unordered_map<Constructor, WeakReference<Object>, Hash> map_;
        std::unordered_map<Object*, Constructor> lut_;

    public:
        SmartReference<Object> get(const Constructor& ctor)
        {
            auto it = map_.find(ctor);
            if (it != map_.end())
                return it->second.lock();
            else
            {
                SmartReference<Object> obj(ctor());
                obj.set_on_delete(&on_ref_delete, this);
                map_.emplace(ctor, obj.weak());
                lut_.emplace(obj.base(), ctor);
                return obj;
            }
        }

        void clear()
        {
            map_.clear();
            lut_.clear();
        }

    private:
        static void on_ref_delete(Object& obj, void* usr)
        {
            reinterpret_cast<SmartReferenceLookup<Object, Constructor>*>(usr)->on_ref_delete_impl(obj);
        }

        void on_ref_delete_impl(Object& obj)
        {
            auto it = lut_.find(&obj);
            map_.erase(it->second);
            lut_.erase(it);
        }
    };
}
