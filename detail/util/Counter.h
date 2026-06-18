#pragma once

#include <unordered_map>
#include <vector>

namespace oly
{
	template<typename T, typename Hash = std::hash<T>, typename Equals = std::equal_to<T>>
	class Counter
	{
		std::unordered_map<T, size_t, Hash, Equals> _map;

	public:
		void increment(const T& obj, size_t by = 1)
		{
			auto it = _map.find(obj);
			if (it != _map.end())
				it->second += by;
			else if (by > 0)
				_map.emplace(obj, by);
		}

		void increment(T&& obj, size_t by = 1)
		{
			auto it = _map.find(obj);
			if (it != _map.end())
				it->second += by;
			else if (by > 0)
				_map.emplace(std::move(obj), by);
		}

		bool decrement(const T& obj)
		{
			auto it = _map.find(obj);
			if (it != _map.end())
			{
				if (--it->second == 0)
					_map.erase(it);
				return true;
			}
			else
				return false;
		}

		size_t count(const T& obj) const
		{
			auto it = _map.find(obj);
			return it != _map.end() ? it->second : 0;
		}

		void clear()
		{
			_map.clear();
		}

		void accumulate(const std::vector<T>& vec)
		{
			for (const T& el : vec)
				increment(el);
		}
	};
}
