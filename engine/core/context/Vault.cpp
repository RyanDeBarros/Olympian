#include "Vault.h"

#include "core/base/Errors.h"

#include "core/context/TickService.h"

#include <unordered_map>

namespace oly::context::internal
{
    VaultKey::VaultKey(size_t i)
        : i(i)
    {
    }

    bool VaultKey::operator==(VaultKey other) const
    {
        return i == other.i;
    }

    VaultKey VaultKey::operator++(int)
    {
        VaultKey v = *this;
        ++i;
        return v;
    }

    VaultKey::operator size_t() const
    {
        return i;
    }
}

template<>
struct std::hash<oly::context::internal::VaultKey>
{
    size_t operator()(oly::context::internal::VaultKey key) const { return (size_t)key; }
};

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

        VaultKey get_next_vault_key()
        {
            static size_t next = 1;
            return VaultKey(next++);
        }

        void vault_set(VaultKey key, imp::box value)
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
