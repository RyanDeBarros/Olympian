#pragma once

#include <set>

namespace oly
{
	template<typename T>
	struct Range
	{
		T initial = T();
		T length = T();

		constexpr T end() const { return initial + length; }
		constexpr bool contains(T in) const { return in >= initial && in < end(); }
		constexpr bool contains(Range<T> sub) const { return contains(sub.initial) && sub.end() <= end(); }
		constexpr bool overlaps(Range<T> other) const { return contains(other.initial) || other.contains(initial); }
		constexpr bool adjacent(Range<T> other) const { return end() == other.initial || other.end() == initial; }
		void merge(const Range<T>& other)
		{
			T new_end = std::max(end(), other.end());
			initial = std::min(initial, other.initial);
			length = new_end - initial;
		}
	};

	template<typename T>
	struct RangeComparator
	{
		constexpr bool operator()(const Range<T>& lhs, const Range<T>& rhs) const
		{
			return lhs.initial < rhs.initial || (lhs.initial == rhs.initial && lhs.length < rhs.length);
		}
	};

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

	template<typename T>
	struct Interval
	{
		T left = T(), right = T();

		T length() const { return right - left; }

		enum class Mode
		{
			CLOSED,
			OPEN,
			RIGHT_OPEN,
			LEFT_OPEN
		};
		template<Mode mode = Mode::CLOSED>
		bool contains(T pt) const
		{
			if constexpr (mode == Mode::CLOSED)
				return left <= pt && pt <= right;
			else if constexpr (mode == Mode::OPEN)
				return left < pt && pt < right;
			else if constexpr (mode == Mode::RIGHT_OPEN)
				return left <= pt && pt < right;
			else
				return left < pt && pt <= right;
		}
	};
}
