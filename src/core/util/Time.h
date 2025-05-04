#pragma once

#include "core/types/Meta.h"

namespace oly
{
	class TimeImpl
	{
		double _now;
		double _delta;

	public:
		template<numeric T>
		T now() const { return (T)_now; }
		template<numeric T>
		T delta() const { return (T)_delta; }

	private:
		friend struct Internal;
		void init() { _now = glfwGetTime(); _delta = 1.0f / 60.0f; }
		void sync() { double n = glfwGetTime(); _delta = n - _now; _now = n; }
	};
	inline TimeImpl TIME;
}
