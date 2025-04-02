#pragma once

#include <stack>

namespace oly
{
	template<std::unsigned_integral T>
	class IDGenerator
	{
		T next = 0;
		std::stack<T> yielded;

	public:
		T gen();
		void yield(T id);
	};

	template<std::unsigned_integral T>
	T IDGenerator<T>::gen()
	{
		if (yielded.empty())
			return next++;
		T id = yielded.top();
		yielded.pop();
		return id;
	}

	template<std::unsigned_integral T>
	void IDGenerator<T>::yield(T id)
	{
		yielded.push(id);
	}
}
