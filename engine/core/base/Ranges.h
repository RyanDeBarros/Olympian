#pragma once

#include <set>

namespace oly
{
    template<typename T>
    struct Range
    {
        T initial = T();
        T length = T();

        constexpr T end() const
        {
            return initial + length;
        }

        constexpr bool contains(T in) const
        {
            return in >= initial && in < end();
        }

        constexpr bool contains(Range<T> sub) const
        {
            return contains(sub.initial) && sub.end() <= end();
        }

        constexpr bool overlaps(Range<T> other) const
        {
            return contains(other.initial) || other.contains(initial);
        }

        constexpr bool adjacent(Range<T> other) const
        {
            return end() == other.initial || other.end() == initial;
        }

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
        std::set<Range<T>, RangeComparator<T>> ranges_;

    public:
        void insert(Range<T> range)
        {
            if (ranges_.empty())
            {
                ranges_.insert(range);
                return;
            }

            auto it = ranges_.lower_bound(range);
            if (it == ranges_.end()) // only possibly overlaps last range
            {
                auto last = ranges_.end();
                --last;
                if (range.overlaps(*last) || range.adjacent(*last))
                {
                    range.merge(*last);
                    ranges_.erase(last);
                }
                
                ranges_.insert(range);
            }
            else if (range.initial < it->initial)
            {
                if (it != ranges_.begin())
                    --it;

                Range<T> m = range;
                while (it != ranges_.end())
                {
                    if (range.end() < it->initial)
                        break;

                    if (range.overlaps(*it) || range.adjacent(*it))
                    {
                        m.merge(*it);
                        it = ranges_.erase(it);
                    }
                    else
                        ++it;
                }
                ranges_.insert(m);
            }
            // else: range is already completely in ranges_
        }

        void clear()
        {
            ranges_.clear();
        }
        
        auto begin() const
        {
            return ranges_.begin();
        }
        
        auto end() const
        {
            return ranges_.end();
        }
    };

    template<typename T>
    struct Interval
    {
        enum class Mode
        {
            Closed,
            Open,
            RightOpen,
            LeftOpen
        };

        T left = T();
        T right = T();

        T length() const
        {
            return right - left;
        }

        template<Mode mode = Mode::Closed>
        bool contains(T pt) const
        {
            if constexpr (mode == Mode::Closed)
                return left <= pt && pt <= right;
            else if constexpr (mode == Mode::Open)
                return left < pt && pt < right;
            else if constexpr (mode == Mode::RightOpen)
                return left <= pt && pt < right;
            else
                return left < pt && pt <= right;
        }

        T clamp(T pt) const
        {
            return pt < left ? left : (pt > right ? right : pt);
        }
    };
}
