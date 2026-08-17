#include "Logger.h"

#include "core/editor/Editor.h"

#include <iostream>

namespace oly::editor
{
    ImU32 LogLevelColor(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Success:
            return imtk::col::success;
        case LogLevel::Warning:
            return imtk::col::warning;
        case LogLevel::Error:
            return imtk::col::error;
        default:
            return imtk::col::white;
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
        return Editor::GetLogger();
    }

    void Logger::Log(LogLevel level, std::string msg)
    {
        Instance()._lines.push_back({ level, std::move(msg) });
    }

    void Logger::LogInfo(std::string msg)
    {
        Log(LogLevel::Info, std::move(msg));
    }
    
    void Logger::LogSuccess(std::string msg)
    {
        Log(LogLevel::Success, std::move(msg));
    }
    
    void Logger::LogWarning(std::string msg)
    {
        Log(LogLevel::Warning, std::move(msg));
    }
    
    void Logger::LogError(std::string msg)
    {
        Log(LogLevel::Error, std::move(msg));
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
