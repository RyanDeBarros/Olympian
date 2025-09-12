#pragma once

#include <unordered_map>

namespace oly
{
	template<typename Key, typename Value, typename KeyHash = std::hash<Key>, typename ValueHash = std::hash<Value>,
		typename KeyEquals = std::equal_to<Key>, typename ValueEquals = std::equal_to<Value>>
	class Bijection
	{
		std::unordered_map<Key, Value, KeyHash, KeyEquals> map;
		std::unordered_map<Value, Key, ValueHash, ValueEquals> lut;

	public:
		Bijection() = default;
		Bijection(const Bijection&) = default;
		Bijection(Bijection&&) noexcept = default;
		Bijection& operator=(const Bijection&) = default;
		Bijection& operator=(Bijection&&) noexcept = default;

	private:
		template<typename Map, typename T, typename U>
		static void map_insert(Map& map, T&& k, U&& v)
		{
			if constexpr (std::is_copy_assignable_v<U>)
				map[std::forward<T>(k)] = std::forward<U>(v);
			else
			{
				auto er = map.find(k);
				if (er != map.end())
					map.erase(er);
				map.emplace(std::forward<T>(k), std::forward<U>(v));
			}
		}

	public:
		void set(const Key& key, const Value& value)
		{
			auto forward_it = map.find(key);
			if (forward_it != map.end())
			{
				if (ValueEquals{}(forward_it->second, value))
					return;
				lut.erase(forward_it->second);
			}

			auto backward_it = lut.find(value);
			if (backward_it != lut.end())
			{
				if (KeyEquals{}(backward_it->second, key))
					return;
				map.erase(backward_it->second);
			}

			map_insert(map, key, value);
			map_insert(lut, value, key);
		}

		void forward_erase(const Key& key)
		{
			auto it = map.find(key);
			if (it != map.end())
			{
				lut.erase(it->second);
				map.erase(it);
			}
		}

		void forward_erase(const typename decltype(map)::const_iterator& where)
		{
			if (where != map.end())
			{
				lut.erase(where->second);
				map.erase(where);
			}
		}

		void backward_erase(const Value& key)
		{
			auto it = lut.find(key);
			if (it != lut.end())
			{
				map.erase(it->second);
				lut.erase(it);
			}
		}

		void backward_erase(const typename decltype(lut)::const_iterator& where)
		{
			if (where != lut.end())
			{
				map.erase(where->second);
				lut.erase(where);
			}
		}

		void clear()
		{
			map.clear();
			lut.clear();
		}

		bool forward_key_exists(const Key& key) const
		{
			return map.count(key);
		}

		bool backward_key_exists(const Value& value) const
		{
			return lut.count(value);
		}

		typename decltype(map)::const_iterator forward_end() const
		{
			return map.end();
		}

		typename decltype(lut)::const_iterator backward_end() const
		{
			return lut.end();
		}

		typename decltype(map)::const_iterator find_forward_iterator(const Key& key) const
		{
			return map.find(key);
		}

		typename decltype(lut)::const_iterator find_backward_iterator(const Value& key) const
		{
			return lut.find(key);
		}

		const Value& get_forward_value(const Key& key) const
		{
			return *map.find(key);
		}

		const Key& get_backward_value(const Value& key) const
		{
			return *lut.find(key);
		}
	};
}
