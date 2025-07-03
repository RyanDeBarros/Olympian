#pragma once

#include "core/base/Errors.h"

#include <vector>

namespace oly
{
	template<typename K, typename V>
	class ContiguousMap
	{
		struct Pair
		{
			K key;
			mutable V value;
		};

		using Iterator = std::vector<Pair>::const_iterator;

		std::vector<Pair> map;

	public:
		Iterator insert(const K& key, const V& value)
		{
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				if (it->key == key)
				{
					it->value = value;
					return it;
				}
			}
			map.push_back({ key, value });
			return map.end() - 1;
		}

		Iterator insert(const K& key, V&& value)
		{
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				if (it->key == key)
				{
					it->value = std::move(value);
					return it;
				}
			}
			map.push_back({ key, std::move(value) });
			return map.end() - 1;
		}

		Iterator insert(K&& key, const V& value)
		{
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				if (it->key == key)
				{
					it->value = value;
					return it;
				}
			}
			map.push_back({ std::move(key), value });
			return map.end() - 1;
		}

		Iterator insert(K&& key, V&& value)
		{
			for (auto it = map.begin(); it != map.end(); ++it)
			{
				if (it->key == key)
				{
					it->value = std::move(value);
					return it;
				}
			}
			map.push_back({ std::move(key), std::move(value) });
			return map.end() - 1;
		}

		V& operator[](const K& key)
		{
			for (const Pair& pair : map)
				if (pair.key == key)
					return pair.value;

			map.push_back({ key, V() });
			return map.back().value;
		}

		const V& get(const K& key) const
		{
			for (const Pair& pair : map)
				if (pair.key == key)
					return pair.value;
			throw Error(ErrorCode::INVALID_ITERATOR);
		}

		V& get(const K& key)
		{
			for (const Pair& pair : map)
				if (pair.key == key)
					return pair.value;
			throw Error(ErrorCode::INVALID_ITERATOR);
		}

		Iterator erase(const K& key)
		{
			return map.erase(find(key));
		}

		Iterator erase(const Iterator& where)
		{
			return map.erase(where);
		}

		Iterator find(const K& key) const
		{
			for (auto it = map.begin(); it != map.end(); ++it)
				if (it->key == key)
					return it;
			return map.end();
		}

		Iterator begin() const
		{
			return map.begin();
		}

		Iterator end() const
		{
			return map.end();
		}

		size_t count(const K& key) const
		{
			for (auto it = map.begin(); it != map.end(); ++it)
				if (it->key == key)
					return 1;
			return 0;
		}

		bool empty() const
		{
			return map.empty();
		}

		size_t size() const
		{
			return map.size();
		}

		void clear()
		{
			map.clear();
		}
	};
}
