#pragma once

#include "core/containers/BlackBox.h"

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

		extern void terminate_vault();

		inline VaultKey get_next_vault_key()
		{
			static size_t next = 1;
			return VaultKey(next++);
		}

		extern void vault_set(VaultKey key, BlackBox&& value);
		extern const BlackBox& vault_get(VaultKey key);
	}

	template<typename Object>
	inline void vault_set(internal::VaultKey key, Object object)
	{
		internal::vault_set(key, BlackBox(std::move(object)));
	}

	template<typename Object>
	inline Object vault_get(internal::VaultKey key)
	{
		return Object(*internal::vault_get(key).cast<Object>());
	}

	extern void vault_free(internal::VaultKey key);
}

#define OLY_NEXT_VAULT_KEY (::oly::context::internal::get_next_vault_key())
