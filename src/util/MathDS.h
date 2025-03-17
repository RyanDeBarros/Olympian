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
	}
}
