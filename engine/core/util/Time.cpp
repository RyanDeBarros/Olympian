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
        void TimeImpl::init()
        {
            raw_delta_ = 1.0 / 60.0;
            raw_now_ = glfwGetTime();
            process();
        }

        void TimeImpl::sync()
        {
            double n = glfwGetTime();
            raw_delta_ = n - raw_now_;
            raw_now_ = n;
            process();
        }

        void TimeImpl::process()
        {
            processed_now_ = raw_now_ * time_scale;
            inv_processed_now_ = 1.0 / processed_now_;

            processed_delta_ = std::min(raw_delta_, frame_length_clip) * time_scale;
            inv_processed_delta_ = 1.0 / processed_delta_;
        }

        void RealTimeImpl::init()
        {
            delta_ = 1.0 / 60.0;
            now_ = glfwGetTime();
        }

        void RealTimeImpl::sync()
        {
            double n = glfwGetTime();
            delta_ = n - now_;
            now_ = n;
            process();
        }

        void RealTimeImpl::process()
        {
            inv_now_ = 1.0 / now_;
            inv_delta_ = 1.0 / delta_;
        }
    }
}
