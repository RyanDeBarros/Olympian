#include "Time.h"

#include "core/base/SimpleMath.h"

namespace oly
{
	namespace time
	{
		tm time_struct()
		{
			const auto now = std::chrono::system_clock::now();
			const auto time = std::chrono::system_clock::to_time_t(now);
#pragma warning(suppress : 4996)
			const auto current_time = std::localtime(&time);
			return *current_time;
		}

		std::ostream& pass_timestamp(std::ostream& os)
		{
			tm time = time_struct();
			return os << std::put_time(&time, "%Y-%m-%d %H:%M:%S");
		}

		std::string timestamp()
		{
			std::stringstream ss;
			pass_timestamp(ss);
			return ss.str();
		}

		int years_since_1900()
		{
			return time_struct().tm_year;
		}

		Month current_month()
		{
			return static_cast<Month>(time_struct().tm_mon);
		}

		Day current_day()
		{
			return static_cast<Day>(time_struct().tm_wday);
		}

		int current_hour()
		{
			return time_struct().tm_hour;
		}

		int epoch_hours()
		{
			return std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		unsigned int mod_epoch_hours()
		{
			return unsigned_mod(epoch_hours(), 24);
		}

		int epoch_minutes()
		{
			return std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		unsigned int mod_epoch_minutes()
		{
			return unsigned_mod(epoch_minutes(), 60);
		}

		int epoch_seconds()
		{
			return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		unsigned int mod_epoch_seconds()
		{
			return unsigned_mod(epoch_seconds(), 60);
		}

		int epoch_milliseconds()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		unsigned int mod_epoch_milliseconds()
		{
			return unsigned_mod(epoch_milliseconds(), 1000);
		}
	}

	namespace internal
	{
		void TimeImpl::process()
		{
			_processed_now = _raw_now * time_scale;
			_inv_processed_now = 1.0 / _processed_now;

			_processed_delta = std::min(_raw_delta, frame_length_clip) * time_scale;
			_inv_processed_delta = 1.0 / _processed_delta;
		}

		void TimeImpl::init()
		{
			_raw_delta = 1.0 / 60.0;
			_raw_now = glfwGetTime();
			process();
		}

		void TimeImpl::sync()
		{
			double n = glfwGetTime();
			_raw_delta = n - _raw_now;
			_raw_now = n;
			process();
		}

		void RealTimeImpl::process()
		{
			_inv_now = 1.0 / _now;
			_inv_delta = 1.0 / _delta;
		}
		void RealTimeImpl::init()
		{
			_delta = 1.0 / 60.0;
			_now = glfwGetTime();
		}

		void RealTimeImpl::sync()
		{
			double n = glfwGetTime();
			_delta = n - _now;
			_now = n;
			process();
		}
	}
}
