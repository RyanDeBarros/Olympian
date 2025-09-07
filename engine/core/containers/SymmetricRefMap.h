#pragma once

#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace oly
{
	template<typename Key, typename Value>
	class SymmetricRefMap
	{
	public:
		struct UnorderedPair
		{
			const Key* k1;
			const Key* k2;

			bool operator==(const UnorderedPair& other) const { return (k1 == other.k1 && k2 == other.k2) || (k1 == other.k2 && k2 == other.k1); }
		};

		struct UnorderedPairHash
		{
			size_t operator()(const UnorderedPair& pair) const { return std::hash<const void*>{}(pair.k1) ^ std::hash<const void*>{}(pair.k2); }
		};

		using MapType = std::unordered_map<UnorderedPair, Value, UnorderedPairHash>;

	private:
		MapType map;
		std::unordered_map<const Key*, std::unordered_set<const Key*>> lut;

		void map_insert(UnorderedPair pair, const Value& value)
		{
			if constexpr (std::is_copy_assignable_v<Value>)
				map[pair] = value;
			else
			{
				auto er = map.find(pair);
				if (er != map.end())
					map.erase(er);
				map.emplace(pair, value);
			}
		}

	public:
		void clear() { map.clear(); lut.clear(); }

		std::optional<Value> get(const Key& k1, const Key& k2) const
		{
			auto it = map.find({ &k1, &k2 });
			if (it != map.end())
				return it->second;
			else
				return std::nullopt;
		}

		Value get_or(const Key& k1, const Key& k2, const Value& default_value)
		{
			auto it = map.find({ &k1, &k2 });
			if (it != map.end())
				return it->second;
			else
			{
				set(k1, k2, default_value);
				return default_value;
			}
		}

		void set(const Key& k1, const Key& k2, const Value& value)
		{
			map_insert({ &k1, &k2 }, value);
			lut[&k1].insert(&k2);
			lut[&k2].insert(&k1);
		}

		void set(const UnorderedPair& pair, const Value& value)
		{
			map_insert(pair, value);
			lut[pair.k1].insert(pair.k2);
			lut[pair.k2].insert(pair.k1);
		}

		void copy_all(const Key& from, const Key& to)
		{
			auto it = lut.find(&from);
			if (it != lut.end())
			{
				auto& lut_og = it->second;
				auto& lut_copy = lut[&to];
				for (const Key* k : lut_og)
				{
					lut_copy.insert(k);
					map_insert({ &to, k }, map.find({ &from, k })->second);
				}
			}
		}

		void replace_all(const Key& at, const Key& with)
		{
			auto it = lut.find(&with);
			if (it != lut.end())
			{
				auto& lut_og = it->second;
				auto& lut_copy = lut[&at];
				for (const Key* k : lut_og)
				{
					lut_copy.insert(k);
					map_insert({ &at, k }, map.find({ &with, k })->second);
					map.erase({ &with, k });
				}
				lut.erase(&with);
			}
		}

		void erase_all(const Key& k)
		{
			auto it = lut.find(&k);
			if (it != lut.end())
			{
				for (const Key* col : it->second)
					map.erase({ &k, col });
				lut.erase(it);
			}
		}
	};
}
