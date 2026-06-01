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
		class AutoRegistry final : public Singleton<AutoRegistry<T>>
		{
			friend class Singleton<AutoRegistry<T>>;

			friend class AutoRegistrable<T>;
			std::unordered_set<T*> _linked;

		public:
			void clear() { _linked.clear(); }
			const std::unordered_set<T*>& linked() const { return _linked; }
		};
	}

	template<typename T>
	class AutoRegistrable
	{
	public:
		AutoRegistrable()
		{
			registry()._linked.insert(static_cast<T*>(this));
		}

		AutoRegistrable(const AutoRegistrable&)
		{
			registry()._linked.insert(static_cast<T*>(this));
		}

		AutoRegistrable(AutoRegistrable&&) noexcept
		{
			registry()._linked.insert(static_cast<T*>(this));
		}

		~AutoRegistrable()
		{
			registry()._linked.erase(static_cast<T*>(this));
		}

		static internal::AutoRegistry<T>& registry() { return internal::AutoRegistry<T>::instance(); }
	};
}
