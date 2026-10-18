#pragma once

#include "external/GL.h"

#include <imp/traits.hpp>

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
            double raw_now_ = 0.;
            double processed_now_ = 0.;
            double inv_processed_now_ = 0.;
            double raw_delta_ = 0.;
            double processed_delta_ = 0.;
            double inv_processed_delta_ = 0.;

        public:
            double frame_length_clip = 0.2;
            double time_scale = 1.;

            template<imp::numeric T = float>
            T now() const { return static_cast<T>(processed_now_); }

            template<imp::numeric T = float>
            T inverse_now() const { return static_cast<T>(inv_processed_now_); }

            template<imp::numeric T = float>
            T delta() const { return static_cast<T>(processed_delta_); }

            template<imp::numeric T = float>
            T inverse_delta() const { return static_cast<T>(inv_processed_delta_); }

            void init();
            void sync();

        private:
            void process();
        };
    }

    inline internal::TimeImpl kTime;

    namespace internal
    {
        class RealTimeImpl
        {
            double now_ = 0.0;
            double inv_now_ = 0.0;
            double delta_ = 0.0;
            double inv_delta_ = 0.0;

        public:
            template<imp::numeric T = float>
            T now() const { return static_cast<T>(now_); }

            template<imp::numeric T = float>
            T inverse_now() const { return static_cast<T>(inv_now_); }

            template<imp::numeric T = float>
            T delta() const { return static_cast<T>(delta_); }

            template<imp::numeric T = float>
            T inverse_delta() const { return static_cast<T>(inv_delta_); }

            void init();
            void sync();

        private:
            void process();
        };
    }

    inline internal::RealTimeImpl kRealTime;
}
