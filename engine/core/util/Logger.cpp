#include "Logger.h"

#include "core/base/SimpleMath.h"
#include "core/util/Time.h"
#include "core/util/IO.h"

#include <iostream>

namespace oly
{
    Logger::Impl::Impl(bool enabled)
        : enabled_(enabled)
    {
    }

    std::string Logger::SourceInfo::file_name(std::source_location location) const
    {
        return location.file_name();
    }

    std::string Logger::SourceInfo::line(std::source_location location) const
    {
        return std::to_string(location.line());
    }

    std::string Logger::SourceInfo::column(std::source_location location) const
    {
        return std::to_string(location.column());
    }

    std::string Logger::SourceInfo::function_name(std::source_location location) const
    {
        return location.function_name();
    }

    std::string Logger::SourceInfo::full_source(std::source_location location) const
    {
        std::stringstream ss;
        ss << location.file_name() << "(" << location.line() << ":" << location.column() << ") `"
            << location.function_name() << "`: ";
        return ss.str();
    }

    void Logger::flush()
    {
        const std::string buf = stream_.str();

        if (target_.console)
        {
            std::cout << buf;
            std::cout.flush();
        }

        if (target_.logfile)
        {
            file_ << buf;
            file_.flush();
        }

        stream_.str(std::string());
        stream_.clear();
    }

    Logger::OpenGL Logger::opengl(GLenum code)
    {
        return OpenGL{ code };
    }

    Logger::GLFW Logger::glfw(int code)
    {
        return GLFW{ code };
    }

    void Logger::start_log(const LoggerOptions& options)
    {
        target_.logfile = options.use_logfile;
        target_.console = options.use_console;

        if (target_.logfile)
        {
            std::stringstream ss;
            tm time = time::time_struct();
            ss << "../logs/Tester_" << std::put_time(&time, "%Y-%m-%d %H-%M-%S") << ".log";
            detail::ResourcePath logfile(ss.str());
            logfile.create_parents();
            file_.open(logfile.get_absolute());
        }

        const char* prefix = "<<< kLog started at ";
        const char* postfix = " >>>";
        auto setw = std::setw(sizeof(prefix) - 1 + 32 + sizeof(postfix) - 1);
        stream_ << std::setfill('-') << setw << "" << '\n' << prefix;
        pass_timestamp();
        stream_ << postfix << '\n' << std::setfill('-') << setw << "" << '\n';
        flush();
    }

    void Logger::end_log()
    {
        flush();

        const char* prefix = "<<<< kLog ended at ";
        const char* postfix = " >>>>";
        auto setw = std::setw(sizeof(prefix) - 1 + 32 + sizeof(postfix) - 1);
        stream_ << std::setfill('-') << setw << "" << '\n' << prefix;
        pass_timestamp();
        stream_ << postfix << '\n' << std::setfill('-') << setw << "" << '\n';
        flush();
    }

    void Logger::pass_timestamp()
    {
        time::pass_timestamp(stream_) << '.' << std::setfill('0') << std::setw(3) << time::mod_epoch_milliseconds();
    }

    void Logger::start(const char* level, bool timestamp, const char* prefix)
    {
        if (timestamp)
        {
            pass_timestamp();
            stream_ << " ";
        }
        if (prefix)
            stream_ << "[" << level << " - " << prefix << "] ";
        else
            stream_ << "[" << level << "] ";
    }

    void Logger::start(const char* level, Logger::OpenGL g)
    {
        pass_timestamp();
        stream_ << " [" << level << " - GL" << g.code << "] ";
    }

    void Logger::start(const char* level, Logger::GLFW g)
    {
        pass_timestamp();
        stream_ << " [" << level << " - GLFW" << g.code << "] ";
    }

    Logger::Impl Logger::untagged(bool timestamp)
    {
        if (timestamp)
        {
            pass_timestamp();
            stream_ << " ";
        }
        return Impl(true);
    }

    Logger::Impl Logger::debug(bool timestamp, const char* prefix)
    {
        if (enable.debug)
            start("DEBUG", timestamp, prefix);

        return Impl(enable.debug);
    }

    Logger::Impl Logger::debug(Logger::OpenGL g)
    {
        if (enable.debug)
            start("DEBUG", g);
        
        return Impl(enable.debug);
    }

    Logger::Impl Logger::debug(Logger::GLFW g)
    {
        if (enable.debug)
            start("DEBUG", g);

        return Impl(enable.debug);
    }

    Logger::Impl Logger::info(bool timestamp, const char* prefix)
    {
        if (enable.info)
            start("INFO", timestamp, prefix);

        return Impl(enable.info);
    }

    Logger::Impl Logger::info(Logger::OpenGL g)
    {
        if (enable.info)
            start("INFO", g);

        return Impl(enable.info);
    }

    Logger::Impl Logger::info(Logger::GLFW g)
    {
        if (enable.info)
            start("INFO", g);

        return Impl(enable.info);
    }

    Logger::Impl Logger::warning(bool timestamp, const char* prefix)
    {
        if (enable.warning)
            start("WARNING", timestamp, prefix);

        return Impl(enable.warning);
    }

    Logger::Impl Logger::warning(Logger::OpenGL g)
    {
        if (enable.warning)
            start("WARNING", g);

        return Impl(enable.warning);
    }

    Logger::Impl Logger::warning(Logger::GLFW g)
    {
        if (enable.warning)
            start("WARNING", g);

        return Impl(enable.warning);
    }

    Logger::Impl Logger::error(bool timestamp, const char* prefix)
    {
        if (enable.error)
            start("ERROR", timestamp, prefix);

        return Impl(enable.error);
    }

    Logger::Impl Logger::error(Logger::OpenGL g)
    {
        if (enable.error)
            start("ERROR", g);

        return Impl(enable.error);
    }

    Logger::Impl Logger::error(Logger::GLFW g)
    {
        if (enable.error)
            start("ERROR", g);

        return Impl(enable.error);
    }

    Logger::Impl Logger::fatal(bool timestamp, const char* prefix)
    {
        if (enable.fatal)
            start("FATAL", timestamp, prefix);

        return Impl(enable.fatal);
    }

    Logger::Impl Logger::fatal(Logger::OpenGL g)
    {
        if (enable.fatal)
            start("FATAL", g);

        return Impl(enable.fatal);
    }

    Logger::Impl Logger::fatal(Logger::GLFW g)
    {
        if (enable.fatal)
            start("FATAL", g);

        return Impl(enable.fatal);
    }

    bool Logger::Enables::at_level(LogLevel level) const
    {
        switch (level)
        {
        case LogLevel::Untagged:
            return true;

        case LogLevel::Debug:
            return debug;

        case LogLevel::Info:
            return info;

        case LogLevel::Warning:
            return warning;

        case LogLevel::Error:
            return error;

        case LogLevel::Fatal:
            return fatal;

        default:
            throw Error(ErrorCode::UnsupportedSwitchCase);
        }
    }

    Logger::Impl Logger::at_level(LogLevel level, bool timestamp, const char* prefix)
    {
        switch (level)
        {
        case LogLevel::Untagged:
            if (!empty_str(prefix))
                throw Error(ErrorCode::UnsupportedSwitchCase);
            return untagged(timestamp);

        case LogLevel::Debug:
            return debug(timestamp, prefix);

        case LogLevel::Info:
            return info(timestamp, prefix);

        case LogLevel::Warning:
            return warning(timestamp, prefix);

        case LogLevel::Error:
            return error(timestamp, prefix);

        case LogLevel::Fatal:
            return fatal(timestamp, prefix);

        default:
            throw Error(ErrorCode::UnsupportedSwitchCase);
        }
    }

    Logger::Impl Logger::at_level(LogLevel level, Logger::OpenGL g)
    {
        switch (level)
        {
        case LogLevel::Debug:
            return debug(g);

        case LogLevel::Info:
            return info(g);
        
        case LogLevel::Warning:
            return warning(g);
        
        case LogLevel::Error:
            return error(g);
        
        case LogLevel::Fatal:
            return fatal(g);
        
        default:
            throw Error(ErrorCode::UnsupportedSwitchCase);
        }
    }

    Logger::Impl Logger::at_level(LogLevel level, Logger::GLFW g)
    {
        switch (level)
        {
        case LogLevel::Debug:
            return debug(g);

        case LogLevel::Info:
            return info(g);

        case LogLevel::Warning:
            return warning(g);

        case LogLevel::Error:
            return error(g);

        case LogLevel::Fatal:
            return fatal(g);

        default:
            throw Error(ErrorCode::UnsupportedSwitchCase);
        }
    }

    namespace internal
    {
        void LogAccess::start_log(const LoggerOptions& options)
        {
            kLog.start_log(options);
        }
        
        void LogAccess::end_log()
        {
            kLog.end_log();
        }

        Logger::Impl LogAccess::untagged(bool timestamp)
        {
            return kLog.untagged(timestamp);
        }

        Logger::Impl LogAccess::debug(bool timestamp, const char* prefix)
        {
            return kLog.debug(timestamp, prefix);
        }
        
        Logger::Impl LogAccess::debug(Logger::OpenGL g)
        {
            return kLog.debug(g);
        }
        
        Logger::Impl LogAccess::debug(Logger::GLFW g)
        {
            return kLog.debug(g);
        }

        Logger::Impl LogAccess::info(bool timestamp, const char* prefix)
        {
            return kLog.info(timestamp, prefix);
        }

        Logger::Impl LogAccess::info(Logger::OpenGL g)
        {
            return kLog.info(g);
        }

        Logger::Impl LogAccess::info(Logger::GLFW g)
        {
            return kLog.info(g);
        }

        Logger::Impl LogAccess::warning(bool timestamp, const char* prefix)
        {
            return kLog.warning(timestamp, prefix);
        }

        Logger::Impl LogAccess::warning(Logger::OpenGL g)
        {
            return kLog.warning(g);
        }

        Logger::Impl LogAccess::warning(Logger::GLFW g)
        {
            return kLog.warning(g);
        }

        Logger::Impl LogAccess::error(bool timestamp, const char* prefix)
        {
            return kLog.error(timestamp, prefix);
        }

        Logger::Impl LogAccess::error(Logger::OpenGL g)
        {
            return kLog.error(g);
        }

        Logger::Impl LogAccess::error(Logger::GLFW g)
        {
            return kLog.error(g);
        }

        Logger::Impl LogAccess::fatal(bool timestamp, const char* prefix)
        {
            return kLog.fatal(timestamp, prefix);
        }

        Logger::Impl LogAccess::fatal(Logger::OpenGL g)
        {
            return kLog.fatal(g);
        }

        Logger::Impl LogAccess::fatal(Logger::GLFW g)
        {
            return kLog.fatal(g);
        }

        Logger::Impl LogAccess::at_level(LogLevel level, bool timestamp, const char* prefix)
        {
            return kLog.at_level(level, timestamp, prefix);
        }

        Logger::Impl LogAccess::at_level(LogLevel level, Logger::OpenGL g)
        {
            return kLog.at_level(level, g);
        }
        
        Logger::Impl LogAccess::at_level(LogLevel level, Logger::GLFW g)
        {
            return kLog.at_level(level, g);
        }
    }

    Logger::Impl operator<<(Logger::Impl impl, Logger::NL)
    {
        return impl.stream('\n');
    }

    Logger::Impl operator<<(Logger::Impl impl, Logger::ENDL)
    {
        impl << kLog.nl;
        kLog.flush();
        return impl;
    }

    Logger::Impl operator<<(Logger::Impl impl, const void* c)
    {
        return impl.stream(c);
    }

    Logger::Impl operator<<(Logger::Impl impl, const char* c)
    {
        return impl.stream(c);
    }

    Logger::Impl operator<<(Logger::Impl impl, const std::string& s)
    {
        return impl.stream(s);
    }

    Logger::Impl operator<<(Logger::Impl impl, const std::string_view s)
    {
        return impl.stream(s);
    }

    Logger::Impl operator<<(Logger::Impl impl, const StringParam& s)
    {
        return impl.stream(s);
    }

    Logger::Impl operator<<(Logger::Impl impl, bool b)
    {
        return impl << (b ? "true" : "false");
    }

    Logger::Impl operator<<(Logger::Impl impl, glm::vec2 v)
    {
        return impl << "(" << v.x << ", " << v.y << ")";
    }

    Logger::Impl operator<<(Logger::Impl impl, glm::vec3 v)
    {
        return impl << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    }

    Logger::Impl operator<<(Logger::Impl impl, glm::vec4 v)
    {
        return impl << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    }

    Logger::Impl operator<<(Logger::Impl impl, glm::mat2 m)
    {
        return impl << "(" << m[0] << ", " << m[1] << ")";
    }

    Logger::Impl operator<<(Logger::Impl impl, glm::mat3 m)
    {
        return impl << "(" << m[0] << ", " << m[1] << ", " << m[2] << ")";
    }
}
