#pragma once

#include "external/GL.h"
#include "core/types/Meta.h"

namespace oly
{
	namespace internal
	{
		class TimeImpl
		{
			double _raw_now = 0.0;
			double _processed_now = 0.0;
			double _inv_processed_now = 0.0;
			double _raw_delta = 0.0;
			double _processed_delta = 0.0;
			double _inv_processed_delta = 0.0;

		public:
			double frame_length_clip = 0.2;
			double time_scale = 1.0;

			template<numeric T = float>
			T now() const { return (T)_processed_now; }
			template<numeric T = float>
			T inverse_now() const { return (T)_inv_processed_now; }
			template<numeric T = float>
			T delta() const { return (T)_processed_delta; }
			template<numeric T = float>
			T inverse_delta() const { return (T)_inv_processed_delta; }

		private:
			void process();

		public:
			void init();
			void sync();
		};
	}
	inline internal::TimeImpl TIME;
}
