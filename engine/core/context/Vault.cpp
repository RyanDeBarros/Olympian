#include "Vault.h"

#include "core/base/Errors.h"

#include <unordered_map>

template<>
struct std::hash<oly::context::internal::VaultKey>
{
	size_t operator()(oly::context::internal::VaultKey key) const { return (size_t)key; }
};

// TODO v7 for multi-threading, use mutexes in all context functions.

namespace oly::context
{
	namespace internal
	{
		std::unordered_map<VaultKey, BlackBox> map;

		void terminate_vault()
		{
			map.clear();
		}

		void vault_set(VaultKey key, BlackBox&& value)
		{
			map[key] = std::move(value);
		}

		const BlackBox& vault_get(VaultKey key)
		{
			auto it = map.find(key);
			if (it != map.end())
				return it->second;
			else
				throw Error(ErrorCode::INVALID_ID);
		}
	}

	void vault_free(internal::VaultKey key)
	{
		internal::map.erase(key);
	}

	bool vault_key_exists(internal::VaultKey key)
	{
		return internal::map.count(key);
	}
}
