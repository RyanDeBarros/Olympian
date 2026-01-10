#pragma once

#include "core/types/Singleton.h"

#include <unordered_set>

namespace oly
{
	template<typename T>
	class AutoRegistrable;

	namespace internal
	{
		template<typename T>
		class AutoRegistry : public Singleton<AutoRegistry<T>>
		{
			friend class Singleton<AutoRegistry<T>>;

			friend class AutoRegistrable<T>;
			std::unordered_set<T*> _tracked;

		public:
			void clear()
			{
				_tracked.clear();
			}

			const std::unordered_set<T*>& tracked() const { return _tracked; }
		};
	}

	template<typename T>
	class AutoRegistrable
	{
	public:
		AutoRegistrable()
		{
			registry()._tracked.insert(static_cast<T*>(this));
		}

		AutoRegistrable(const AutoRegistrable&)
		{
			registry()._tracked.insert(static_cast<T*>(this));
		}

		AutoRegistrable(AutoRegistrable&&) noexcept
		{
			registry()._tracked.insert(static_cast<T*>(this));
		}

		~AutoRegistrable()
		{
			registry()._tracked.erase(static_cast<T*>(this));
		}

		static internal::AutoRegistry<T>& registry() { return internal::AutoRegistry<T>::instance(); }
	};
}
