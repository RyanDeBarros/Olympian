#pragma once

#include "core/types/SmartReference.h"

#include <unordered_map>

namespace oly
{
	template<typename Object, typename Constructor>
	class SmartReferenceLookup
	{
		struct Hash
		{
			size_t operator()(Constructor ctor) const { return ctor.hash(); }
		};

		std::unordered_map<Constructor, WeakReference<Object>, Hash> map;
		std::unordered_map<Object*, Constructor> lut;

		static void on_ref_delete(Object& obj, void* usr)
		{
			reinterpret_cast<SmartReferenceLookup<Object, Constructor>*>(usr)->on_ref_delete_impl(obj);
		}

		void on_ref_delete_impl(Object& obj)
		{
			auto it = lut.find(&obj);
			map.erase(it->second);
			lut.erase(it);
		}

	public:
		SmartReference<Object> get(const Constructor& ctor)
		{
			auto it = map.find(ctor);
			if (it != map.end())
				return it->second.lock();
			else
			{
				SmartReference<Object> obj(ctor());
				obj.set_on_delete(&on_ref_delete, this);
				map.emplace(ctor, obj.weak());
				lut.emplace(obj.base(), ctor);
				return obj;
			}
		}

		void clear()
		{
			map.clear();
			lut.clear();
		}
	};
}
