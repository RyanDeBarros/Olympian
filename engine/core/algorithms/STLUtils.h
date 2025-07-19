#pragma once

#include <stack>

namespace oly
{
	template<typename T>
	inline void clear_stack(std::stack<T>& stack)
	{
		while (!stack.empty())
			stack.pop();
	}
}
