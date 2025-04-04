#pragma once

#include <assert.h>
#include <set>

#include "util/General.h"

namespace oly
{
	// This implementation of a free-space tracker is strict in the sense that reserved ranges must overlap exactly with free space, and released ranges must not overlap at all with free space.
	template<std::integral T>
	class StrictFreeSpaceTracker
	{
		std::set<Range<T>, RangeComparator<T>> free_space;

	public:
		StrictFreeSpaceTracker(Range<T> free_space_range);

		void reserve(Range<T> range);
		void release(Range<T> range);
		bool next_free(T length, Range<T>& range);
	};

	template<std::integral T>
	inline StrictFreeSpaceTracker<T>::StrictFreeSpaceTracker(Range<T> free_space_range)
	{
		free_space.insert(free_space_range);
	}

	template<std::integral T>
	inline void StrictFreeSpaceTracker<T>::reserve(Range<T> range)
	{
		assert(!free_space.empty());
		if (range.length == 0)
			return;

		auto lb = free_space.lower_bound(range);
		if (lb == free_space.end()) // last free range 
			--lb;
		else if (range.initial < lb->initial) // lb is subsequent free range
			--lb;
		assert(lb->contains(range));

		if (lb->initial == range.initial)
		{
			if (lb->end() == range.end()) // exact range
				free_space.erase(lb);
			else // left part of range
			{
				T length = lb->length;
				free_space.erase(lb);
				free_space.insert({ range.end(), T(length - range.length) });
			}
		}
		else
		{
			if (lb->end() == range.end()) // right part of range
			{
				T initial = lb->initial;
				free_space.erase(lb);
				free_space.insert({ initial, T(range.initial - initial) });
			}
			else // middle of range
			{
				T initial = lb->initial;
				T end = lb->end();
				free_space.erase(lb);
				free_space.insert({ initial, T(range.initial - initial) });
				free_space.insert({ range.end(), T(end - range.end()) });
			}
		}
	}

	template<std::integral T>
	inline void StrictFreeSpaceTracker<T>::release(Range<T> range)
	{
		if (range.length == 0)
			return;
		if (free_space.empty())
		{
			free_space.insert(range);
			return;
		}

		auto lb = free_space.lower_bound(range);
		if (lb == free_space.end())
		{
			--lb;
			assert(!lb->contains(range.initial));
			if (lb->end() == range.initial)
			{
				T initial = lb->initial;
				free_space.erase(lb);
				free_space.insert({ initial, T(range.end() - initial) });
			}
			else
				free_space.insert(range);
		}
		else if (lb == free_space.begin())
		{
			assert(range.initial < lb->initial && range.end() <= lb->initial);
			if (range.end() == lb->initial)
			{
				T end = lb->end();
				free_space.erase(lb);
				free_space.insert({ range.initial, T(end - range.initial) });
			}
			else
				free_space.insert(range);
		}
		else
		{
			assert(range.initial < lb->initial);
			auto pr = lb;
			--pr;
			assert(pr->end() <= range.initial);
			assert(range.end() <= lb->initial);
			if (pr->end() == range.initial)
			{
				T initial = pr->initial;
				if (range.end() == lb->initial)
				{
					T end = lb->end();
					++lb;
					free_space.erase(pr, lb);
					free_space.insert({ initial, T(end - initial) });
				}
				else
				{
					free_space.erase(pr);
					free_space.insert({ initial, T(range.end() - initial) });
				}
			}
			else
			{
				if (range.end() == lb->initial)
				{
					T end = lb->end();
					free_space.erase(lb);
					free_space.insert({ range.initial, T(end - range.initial) });
				}
				else
					free_space.insert(range);
			}
		}
	}

	template<std::integral T>
	inline bool StrictFreeSpaceTracker<T>::next_free(T length, Range<T>& range)
	{
		auto end = free_space.end();
		for (auto iter = free_space.begin(); iter != end; ++iter)
		{
			if (iter->length >= length)
			{
				range.initial = iter->initial;
				range.length = length;
				return true;
			}
		}
		return false;
	}
}
