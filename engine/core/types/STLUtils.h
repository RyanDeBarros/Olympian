#pragma once

#include <vector>

namespace oly
{
	template<typename T>
	inline void vector_erase(std::vector<T>& vec, const T& el)
	{
		vec.erase(std::find(vec.begin(), vec.end(), el));
	}
}
