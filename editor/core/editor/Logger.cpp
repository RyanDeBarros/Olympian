#include "Logger.h"

#include "core/editor/Editor.h"
#include "core/Colors.h"

#include <iostream>

namespace oly::editor
{
    ImU32 LogLevelColor(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Success:
            return Color::Success;
        case LogLevel::Warning:
            return Color::Warning;
        case LogLevel::Error:
            return Color::Error;
        default:
            return Color::White;
        }
    }

    const char* LogLevelPrefix(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Info:
            return "[Info]";
        case LogLevel::Success:
            return "[Success]";
        case LogLevel::Warning:
            return "[Warning]";
        case LogLevel::Error:
            return "[Error]";
        default:
            return "";
        }
    }

    Logger& Logger::Instance()
    {
        return Editor::Instance().GetLogger();
    }

    void Logger::Log(LogLevel level, const char* msg)
    {
        Log(level, std::string(msg));
    }

    void Logger::Log(LogLevel level, const std::string& msg)
    {
        Log(level, std::string(msg));
    }

    void Logger::Log(LogLevel level, std::string&& msg)
    {
        _lines.push_back({ level, std::move(msg) });
    }

    void Logger::ClearLog()
    {
        _lines.clear();
    }

    const std::vector<LogEntry>& Logger::Lines() const
    {
        return _lines;
    }
}
