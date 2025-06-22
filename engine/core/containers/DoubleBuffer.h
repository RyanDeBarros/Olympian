#pragma once

#include <vector>

namespace oly
{
	template<typename T>
	class DoubleBuffer
	{
		std::vector<T> _first, _second;

	public:
		const std::vector<T>& front() const { return _first; }
		std::vector<T>& front() { return _first; }
		const std::vector<T>& back() const { return _second; }
		std::vector<T>& back() { return _second; }

		DoubleBuffer& swap() { std::swap(_first, _second); return *this; }
	};
}
