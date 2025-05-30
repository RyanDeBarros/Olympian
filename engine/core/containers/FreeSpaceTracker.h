#pragma once

#include "core/containers/Ranges.h"
#include "core/base/Assert.h"

namespace oly
{
	// This implementation of a free-space tracker is strict in the sense that reserved ranges must overlap exactly with free space, and released ranges must not overlap at all with free space.
	template<std::integral T>
	class StrictFreeSpaceTracker
	{
		std::set<Range<T>, RangeComparator<T>> free_space;
		Range<T> global;

	public:
		StrictFreeSpaceTracker(Range<T> free_space_range);

		void reserve(Range<T> range);
		void release(Range<T> range);
		bool next_free(T length, Range<T>& range);
		void extend_rightward(T end);
		void extend_leftward(T initial);
	};

	template<std::integral T>
	inline StrictFreeSpaceTracker<T>::StrictFreeSpaceTracker(Range<T> free_space_range)
		: global(free_space_range)
	{
		free_space.insert(free_space_range);
	}

	template<std::integral T>
	inline void StrictFreeSpaceTracker<T>::reserve(Range<T> range)
	{
		OLY_ASSERT(!free_space.empty());
		if (range.length == 0)
			return;

		auto lb = free_space.lower_bound(range);
		if (lb == free_space.end()) // last free range 
			--lb;
		else if (range.initial < lb->initial) // lb is subsequent free range
			--lb;
		OLY_ASSERT(lb->contains(range));

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
			OLY_ASSERT(!lb->contains(range.initial));
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
			OLY_ASSERT(range.initial < lb->initial && range.end() <= lb->initial);
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
			OLY_ASSERT(range.initial < lb->initial);
			auto pr = lb;
			--pr;
			OLY_ASSERT(pr->end() <= range.initial);
			OLY_ASSERT(range.end() <= lb->initial);
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

	template<std::integral T>
	inline void StrictFreeSpaceTracker<T>::extend_rightward(T end)
	{
		if (end <= global.end())
			return;

		if (free_space.empty())
			free_space.insert({ global.end(), end - global.end() });
		else
		{
			auto last = free_space.end();
			--last;
			if (last->end() < global.end())
				free_space.insert({ global.end(), end - global.end() });
			else
			{
				Range<T> insert{ last->initial, end - last->initial };
				free_space.erase(last);
				free_space.insert(insert);
			}
		}
		global.length = end - global.initial;
	}

	template<std::integral T>
	inline void StrictFreeSpaceTracker<T>::extend_leftward(T initial)
	{
		if (initial >= global.initial)
			return;
		
		if (free_space.empty())
			free_space.insert({ initial, global.initial - initial });
		else
		{
			auto first = free_space.begin();
			if (first->initial > global.initial)
				free_space.insert({ initial, global.initial - initial });
			else
			{
				Range<T> insert{ initial, first->end() - initial };
				free_space.erase(first);
				free_space.insert(insert);
			}
		}
		global.length += global.initial - initial;
		global.initial = initial;
	}
}
