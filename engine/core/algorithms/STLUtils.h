#pragma once

#include <stack>
#include <set>
#include <vector>
#include <string_view>

#include "core/base/Errors.h"
#include "core/types/Meta.h"

namespace oly::algo
{
	template<typename T>
	inline void clear_stack(std::stack<T>& stack)
	{
		while (!stack.empty())
			stack.pop();
	}

	template<typename T, typename U, typename DistanceFunc>
	const T& find_closest(const std::set<T>& set, const U& to, DistanceFunc dist)
	{
		if (set.empty())
			throw Error(ErrorCode::EMPTY_DATA_STRUCTURE);
		else if (set.size() == 1)
			return *set.begin();

		auto it = set.lower_bound(to);
		if (it == set.end())
			it = set.begin();

		auto prev = it != set.begin() ? std::prev(it) : std::prev(set.end());
		return dist(*prev, to) < dist(to, *it) ? *prev : *it;
	}

	template<typename T, typename DistanceFunc>
	T& find_closest(std::set<T>& set, const T& to, DistanceFunc dist)
	{
		return const_cast<T&>(find_closest(const_cast<const std::set<T>&>(set), to, dist));
	}

	template<typename T>
	const T& find_closest(const std::set<T>& set, const T& to)
	{
		return find_closest(set, to, [](const T& a, const T& b) { return b - a; });
	}

	template<typename T>
	T& find_closest(std::set<T>& set, const T& to)
	{
		return const_cast<T&>(find_closest(const_cast<const std::set<T>&>(set), to));
	}

	template<typename T>
	const T& find_closest_with_mod(const std::set<T>& set, const T& to, const T& mod)
	{
		return find_closest(set, to, [&mod](const T& a, const T& b) { return (b + mod - a) % mod; });
	}

	template<typename T>
	T& find_closest_with_mod(std::set<T>& set, const T& to, const T& mod)
	{
		return const_cast<T&>(find_closest_with_mod(const_cast<const std::set<T>&>(set), to, mod));
	}

	extern std::vector<std::string_view> split(std::string_view sv, char delimiter);

	extern std::string ltrim(std::string&& str);
	inline std::string ltrim(const std::string& str) { return ltrim(dupl(str)); }
	inline void ltrim(std::string& str) { str = ltrim(std::move(str)); }
	extern std::string rtrim(std::string&& str);
	inline std::string rtrim(const std::string& str) { return rtrim(dupl(str)); }
	inline void rtrim(std::string& str) { str = rtrim(std::move(str)); }
	extern std::string trim(std::string&& str);
	inline std::string trim(const std::string& str) { return trim(dupl(str)); }
	inline void trim(std::string& str) { str = trim(std::move(str)); }
	extern std::string to_lower(std::string&& str);
	inline std::string to_lower(const std::string& str) { return to_lower(dupl(str)); }
	inline void to_lower(std::string& str) { str = to_lower(std::move(str)); }
	extern std::string to_upper(std::string&& str);
	inline std::string to_upper(const std::string& str) { return to_upper(dupl(str)); }
	inline void to_upper(std::string& str) { str = to_upper(std::move(str)); }
}
