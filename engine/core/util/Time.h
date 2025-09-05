#pragma once

#include "external/GL.h"
#include "core/types/Meta.h"

namespace oly
{
	namespace internal
	{
		class TimeImpl
		{
			double _now = 0.0;
			double _inv_now = 0.0;
			double _delta = 0.0;
			double _inv_delta = 0.0;
			double _lagged = 0.0;

		public:
			double frame_length_clip = 0.2;

			template<numeric T = float>
			T now() const { return (T)_now; }
			template<numeric T = float>
			T inverse_now() const { return (T)_inv_now; }
			template<numeric T = float>
			T delta() const { return (T)_delta; }
			template<numeric T = float>
			T inverse_delta() const { return (T)_inv_delta; }
			template<numeric T = float>
			T lagged() const { return (T)_lagged; }

			void init();
			void sync();
		};
	}
	inline internal::TimeImpl TIME;
}
