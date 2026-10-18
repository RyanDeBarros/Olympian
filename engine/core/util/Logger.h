#pragma once

#include "core/util/LogLevel.h"
#include "core/util/StringParam.h"

#include "external/GL.h"
#include "external/GLM.h"

#include <imp/soft_singleton.hpp>
#include <imp/traits.hpp>

#include <fstream>
#include <optional>
#include <source_location>
#include <sstream>
#include <string_view>

namespace oly
{
    namespace internal
    {
        struct LogAccess;
    }

    struct LoggerOptions
    {
        bool use_console = true;
        bool use_logfile = true;
        std::optional<size_t> max_prior_log_files = 20;
        std::optional<size_t> max_prior_log_bytes = std::nullopt;
    };

    class Logger final
        : public imp::soft_singleton<Logger>
    {
        friend class imp::soft_singleton<Logger>;
        friend struct internal::LogAccess;
        friend class Impl;

        struct OpenGL
        {
            GLenum code;
        };

        struct GLFW
        {
            int code;
        };

    public:
        struct Enables
        {
            bool debug = false;
            bool info = true;
            bool warning = true;
            bool error = true;
            bool fatal = true;

            bool at_level(LogLevel level) const;
        };

        class Impl
        {
            friend class Logger;

        private:
            bool enabled_;

            Impl(bool enabled);

        public:
            template<typename T>
            Impl stream(T&& obj)
            {
                if (enabled_)
                    Logger::instance().stream_ << std::forward<T>(obj);

                return *this;
            }
        };

        struct SourceInfo
        {
            std::string file_name(std::source_location location = std::source_location::current()) const;
            std::string line(std::source_location location = std::source_location::current()) const;
            std::string column(std::source_location location = std::source_location::current()) const;
            std::string function_name(std::source_location location = std::source_location::current()) const;

            std::string full_source(std::source_location location = std::source_location::current()) const;
        };

    private:
        std::stringstream stream_;
        std::ofstream file_;

        struct
        {
            bool console = true;
            bool logfile = true;
        } target_;

    public:
        Enables enable;
        struct NL {} nl;
        struct ENDL {} endl;
        SourceInfo source_info;

    public:
        void flush();

        OpenGL opengl(GLenum code);
        GLFW glfw(int code);

    private:
        void start_log(const LoggerOptions& options);
        void end_log();

        void pass_timestamp();

        void start(const char* level, bool timestamp, const char* prefix);
        void start(const char* level, OpenGL);
        void start(const char* level, GLFW);

        Impl untagged(bool timestamp = false);

        Impl debug(bool timestamp = false, const char* prefix = nullptr);
        Impl debug(OpenGL);
        Impl debug(GLFW);

        Impl info(bool timestamp = false, const char* prefix = nullptr);
        Impl info(OpenGL);
        Impl info(GLFW);

        Impl warning(bool timestamp = false, const char* prefix = nullptr);
        Impl warning(OpenGL);
        Impl warning(GLFW);

        Impl error(bool timestamp = false, const char* prefix = nullptr);
        Impl error(OpenGL);
        Impl error(GLFW);

        Impl fatal(bool timestamp = false, const char* prefix = nullptr);
        Impl fatal(OpenGL);
        Impl fatal(GLFW);

        Impl at_level(LogLevel level, bool timestamp = false, const char* prefix = nullptr);
        Impl at_level(LogLevel level, OpenGL);
        Impl at_level(LogLevel level, GLFW);
    };

    inline Logger& kLog = Logger::instance();

    namespace internal
    {
        struct LogAccess
        {
            static void start_log(const LoggerOptions& options);
            static void end_log();

            static Logger::Impl untagged(bool timestamp = false);

            static Logger::Impl debug(bool timestamp = false, const char* prefix = nullptr);
            static Logger::Impl debug(Logger::OpenGL g);
            static Logger::Impl debug(Logger::GLFW g);

            static Logger::Impl info(bool timestamp = false, const char* prefix = nullptr);
            static Logger::Impl info(Logger::OpenGL g);
            static Logger::Impl info(Logger::GLFW g);

            static Logger::Impl warning(bool timestamp = false, const char* prefix = nullptr);
            static Logger::Impl warning(Logger::OpenGL g);
            static Logger::Impl warning(Logger::GLFW g);

            static Logger::Impl error(bool timestamp = false, const char* prefix = nullptr);
            static Logger::Impl error(Logger::OpenGL g);
            static Logger::Impl error(Logger::GLFW g);

            static Logger::Impl fatal(bool timestamp = false, const char* prefix = nullptr);
            static Logger::Impl fatal(Logger::OpenGL g);
            static Logger::Impl fatal(Logger::GLFW g);

            static Logger::Impl at_level(LogLevel level, bool timestamp = false, const char* prefix = nullptr);
            static Logger::Impl at_level(LogLevel level, Logger::OpenGL g);
            static Logger::Impl at_level(LogLevel level, Logger::GLFW g);
        };
    }

    extern Logger::Impl operator<<(Logger::Impl, Logger::NL);
    extern Logger::Impl operator<<(Logger::Impl, Logger::ENDL);

    extern Logger::Impl operator<<(Logger::Impl, const void*);
    extern Logger::Impl operator<<(Logger::Impl, const char*);
    extern Logger::Impl operator<<(Logger::Impl, const std::string&);
    extern Logger::Impl operator<<(Logger::Impl, const std::string_view);
    extern Logger::Impl operator<<(Logger::Impl, const StringParam&);
    extern Logger::Impl operator<<(Logger::Impl, bool);

    template<imp::numeric T>
    inline Logger::Impl operator<<(Logger::Impl impl, T v)
    {
        return impl << std::to_string(v);
    }

    extern Logger::Impl operator<<(Logger::Impl, glm::vec2);
    extern Logger::Impl operator<<(Logger::Impl, glm::vec3);
    extern Logger::Impl operator<<(Logger::Impl, glm::vec4);
    extern Logger::Impl operator<<(Logger::Impl, glm::mat2);
    extern Logger::Impl operator<<(Logger::Impl, glm::mat3);
}

#define OLY_LOG(...) oly::internal::LogAccess::untagged(__VA_ARGS__)
#define OLY_LOG_DEBUG(...) if (!(oly::kLog.enable.debug)) ; else oly::internal::LogAccess::debug(__VA_ARGS__)
#define OLY_LOG_INFO(...) if (!(oly::kLog.enable.info)) ; else oly::internal::LogAccess::info(__VA_ARGS__)
#define OLY_LOG_WARNING(...) if (!(oly::kLog.enable.warning)) ; else oly::internal::LogAccess::warning(__VA_ARGS__)
#define OLY_LOG_ERROR(...) if (!(oly::kLog.enable.error)) ; else oly::internal::LogAccess::error(__VA_ARGS__)
#define OLY_LOG_FATAL(...) if (!(oly::kLog.enable.fatal)) ; else oly::internal::LogAccess::fatal(__VA_ARGS__)
#define OLY_LOG_AT_LEVEL(level, ...) if (!(oly::kLog.enable.at_level(level))) ; else oly::internal::LogAccess::at_level(level, __VA_ARGS__)

#define _OLY_ENGINE_LOG_DEBUG(category) OLY_LOG_DEBUG(true, category) << oly::kLog.source_info.full_source()
#define _OLY_ENGINE_LOG_WARNING(category) OLY_LOG_WARNING(true, category) << oly::kLog.source_info.full_source()
#define _OLY_ENGINE_LOG_ERROR(category) OLY_LOG_ERROR(true, category) << oly::kLog.source_info.full_source()
#define _OLY_ENGINE_LOG_FATAL(category) OLY_LOG_FATAL(true, category) << oly::kLog.source_info.full_source()
#define _OLY_ENGINE_LOG_AT_LEVEL(category) OLY_LOG_AT_LEVEL(level, true, category) << oly::kLog.source_info.full_source()
