#pragma once

#include "core/base/Errors.h"

#include <imp/box.hpp>

namespace oly::context
{
	namespace internal
	{
		struct VaultKey
		{
			size_t i;

			explicit VaultKey(size_t i) : i(i) {}
			VaultKey operator++(int) { VaultKey v = *this; ++i; return v; }
			explicit operator size_t () const { return i; }
			bool operator==(VaultKey other) const { return i == other.i; }
		};

		extern void init_vault();

		inline VaultKey get_next_vault_key()
		{
			static size_t next = 1;
			return VaultKey(next++);
		}

		extern void vault_set(VaultKey key, imp::box&& value);
		extern const imp::box& vault_get(VaultKey key);
	}

	template<typename Object>
	inline void vault_set(internal::VaultKey key, Object&& object)
	{
		internal::vault_set(key, imp::forward_to_box(std::forward<Object>(object)));
	}

	template<typename Object>
	inline Object vault_get(internal::VaultKey key)
	{
        if (auto obj = internal::vault_get(key).as<Object>())
            return *obj;
        else
            throw Error(ErrorCode::InvalidType);
	}

	extern void vault_free(internal::VaultKey key);
	extern bool vault_key_exists(internal::VaultKey key);

	template<typename Func>
	inline auto vault_prototype(internal::VaultKey key, Func&& generate_first)
	{
		using Object = std::decay_t<decltype(std::invoke(std::forward<Func>(generate_first)))>;

		if (vault_key_exists(key))
			return vault_get<Object>(key);
		else
		{
			Object object = std::invoke(std::forward<Func>(generate_first));
			vault_set(key, object);
			return object;
		}
	}
}

#define OLY_NEXT_VAULT_KEY ::oly::context::internal::get_next_vault_key()
