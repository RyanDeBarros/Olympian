#pragma once

#include <vector>

namespace oly
{
	template<typename T>
	struct DoubleBuffer
	{
		std::vector<T> front, back;

		DoubleBuffer& swap() { std::swap(front, back); return *this; }
	};
}
