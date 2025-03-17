#pragma once

#include <vector>

namespace oly
{
	namespace math
	{
		template<typename T>
		inline std::vector<T> fill_linear(T initial, T step, size_t num)
		{
			std::vector<T> vec;
			vec.reserve(num);
			T el = initial;
			for (size_t i = 0; i < num; ++i)
			{
				vec.push_back(el);
				el += step;
			}
			return vec;
		}

		template<typename Iterator, typename T>
		inline void fill_linear(Iterator begin, Iterator end, T initial, T step)
		{
			T el = initial;
			for (Iterator it = begin; it != end; ++it)
			{
				*it = el;
				el += step;
			}
		}

		template<std::integral Index>
		class IndexBijection
		{
		public:
			typedef Index DomainIndex;
			typedef Index RangeIndex;

		private:
			std::vector<RangeIndex> forward;
			std::vector<DomainIndex> backward;

		public:
			IndexBijection(size_t size)
			{
				forward = fill_linear<DomainIndex>(0, 1, size);
				backward = fill_linear<RangeIndex>(0, 1, size);
			}

			RangeIndex range_of(DomainIndex d) const { return forward[d]; }
			DomainIndex domain_of(RangeIndex r) const { return backward[r]; }

			void swap_domain(DomainIndex d1, DomainIndex d2)
			{
				RangeIndex r1 = forward[d1];
				RangeIndex r2 = forward[d2];
				backward[r1] = d2;
				backward[r2] = d1;
				forward[d1] = r2;
				forward[d2] = r1;
			}

			void swap_range(RangeIndex r1, RangeIndex r2)
			{
				DomainIndex d1 = backward[r1];
				DomainIndex d2 = backward[r2];
				forward[d1] = r2;
				forward[d2] = r1;
				backward[r1] = d2;
				backward[r2] = d1;
			}
		};
	}
}
