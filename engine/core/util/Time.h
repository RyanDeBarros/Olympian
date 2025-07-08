#pragma once

#include "external/GL.h"
#include "core/types/Meta.h"

namespace oly
{
	namespace internal
	{
		class TimeImpl
		{
			double _now;
			double _delta;

		public:
			template<numeric T = float>
			T now() const { return (T)_now; }
			template<numeric T = float>
			T delta() const { return (T)_delta; }

			void init() { _now = glfwGetTime(); _delta = 1.0f / 60.0f; }
			void sync() { double n = glfwGetTime(); _delta = n - _now; _now = n; }
		};
	}
	inline internal::TimeImpl TIME;
}
