#pragma once

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
