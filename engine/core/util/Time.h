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
			double _inv_now;
			double _delta;
			double _inv_delta;

		public:
			template<numeric T = float>
			T now() const { return (T)_now; }
			template<numeric T = float>
			T inverse_now() const { return (T)_inv_now; }
			template<numeric T = float>
			T delta() const { return (T)_delta; }
			template<numeric T = float>
			T inverse_delta() const { return (T)_inv_delta; }

			void init() { _now = glfwGetTime(); _delta = 1.0f / 60.0f; _inv_now = 1.0f / _now; _inv_delta = 1.0f / _delta; }
			void sync() { double n = glfwGetTime(); _delta = n - _now; _now = n; _inv_now = 1.0f / _now; _inv_delta = 1.0f / _delta; }
		};
	}
	inline internal::TimeImpl TIME;
}
