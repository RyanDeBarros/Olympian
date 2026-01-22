#pragma once

#include "external/GL.h"
#include "core/types/Meta.h"

#include <chrono>

namespace oly
{
	namespace time
	{
		enum Month
		{
			January,
			February,
			March,
			April,
			May,
			June,
			July,
			August,
			September,
			October,
			November,
			December
		};

		enum Day
		{
			Sunday,
			Monday,
			Tuesday,
			Wednesday,
			Thursday,
			Friday,
			Saturday
		};

		extern tm time_struct();
		extern std::ostream& pass_timestamp(std::ostream& os);
		extern std::string timestamp();
		extern int years_since_1900();
		extern Month current_month();
		extern Day current_day();
		extern int current_hour();
		extern int epoch_hours();
		extern unsigned int mod_epoch_hours();
		extern int epoch_minutes();
		extern unsigned int mod_epoch_minutes();
		extern int epoch_seconds();
		extern unsigned int mod_epoch_seconds();
		extern int epoch_milliseconds();
		extern unsigned int mod_epoch_milliseconds();
	}

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

	namespace internal
	{
		class RealTimeImpl
		{
			double _now = 0.0;
			double _inv_now = 0.0;
			double _delta = 0.0;
			double _inv_delta = 0.0;

		public:
			template<numeric T = float>
			T now() const { return (T)_now; }
			template<numeric T = float>
			T inverse_now() const { return (T)_inv_now; }
			template<numeric T = float>
			T delta() const { return (T)_delta; }
			template<numeric T = float>
			T inverse_delta() const { return (T)_inv_delta; }

		private:
			void process();

		public:
			void init();
			void sync();
		};
	}

	inline internal::RealTimeImpl REAL_TIME;
}
