#include "Vault.h"

#include "core/base/Errors.h"
#include "core/context/TickService.h"

#include <unordered_map>

template<>
struct std::hash<oly::context::internal::VaultKey>
{
	size_t operator()(oly::context::internal::VaultKey key) const { return (size_t)key; }
};

// TODO v14 for multi-threading, use mutexes in all context functions.

namespace oly::context
{
	namespace internal
	{
		std::unordered_map<VaultKey, imp::box> map;

		struct VaultOnTerminate
		{
			void operator()() const
			{
				map.clear();
			}
		};

		void init_vault()
		{
			SingletonTickService<TickPhase::None, void, TerminatePhase::Vault, VaultOnTerminate>::instance();
		}

		void vault_set(VaultKey key, imp::box&& value)
		{
			map[key] = std::move(value);
		}

		const imp::box& vault_get(VaultKey key)
		{
			auto it = map.find(key);
			if (it != map.end())
				return it->second;
			else
				throw Error(ErrorCode::InvalidID);
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
