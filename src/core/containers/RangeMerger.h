#pragma once

#include "core/base/Intervals.h"

namespace oly
{
	template<typename T>
	class RangeMerger
	{
		std::set<Range<T>, RangeComparator<T>> ranges;

	public:
		void insert(Range<T> range)
		{
			if (ranges.empty())
			{
				ranges.insert(range);
				return;
			}

			auto it = ranges.lower_bound(range);
			if (it == ranges.end()) // only possibly overlaps last range
			{
				auto last = ranges.end();
				--last;
				if (range.overlaps(*last) || range.adjacent(*last))
				{
					range.merge(*last);
					ranges.erase(last);
				}
				ranges.insert(range);
			}
			else if (range.initial < it->initial)
			{
				if (it != ranges.begin())
					--it;
				Range<T> m = range;
				while (it != ranges.end())
				{
					if (range.end() < it->initial)
						break;
					if (range.overlaps(*it) || range.adjacent(*it))
					{
						m.merge(*it);
						it = ranges.erase(it);
					}
					else
						++it;
				}
				ranges.insert(m);
			}
			// else range is already completely in ranges
		}
		void clear() { ranges.clear(); }
		auto begin() const { return ranges.begin(); }
		auto end() const { return ranges.end(); }
	};
}
