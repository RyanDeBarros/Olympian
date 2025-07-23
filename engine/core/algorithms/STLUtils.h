#pragma once

#include <stack>
#include <set>

#include "core/base/Errors.h"

namespace oly
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
}
