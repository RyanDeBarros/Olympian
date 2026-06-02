#include "Logger.h"

#include "core/Editor.h"

#include <iostream>

namespace oly::editor
{
    // TODO v7 log panel - color lines by log level

    ImU32 LogLevelColor(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Success:
            return IM_COL32(0, 255, 0, 255);
        case LogLevel::Warning:
            return IM_COL32(255, 255, 0, 255);
        case LogLevel::Error:
            return IM_COL32(255, 0, 0, 255);
        default:
            return IM_COL32(255, 255, 255, 255);
        }
    }

    Logger& Logger::Instance()
    {
        return Editor::Instance().GetLogger();
    }

    void Logger::Log(LogLevel level, const char* msg)
    {
        switch (level)
        {
        case LogLevel::Info:
            LogInfo(msg);
            break;
        case LogLevel::Success:
            LogSuccess(msg);
            break;
        case LogLevel::Warning:
            LogWarning(msg);
            break;
        case LogLevel::Error:
            LogError(msg);
            break;
        }
    }

    void Logger::LogInfo(const char* info)
    {
        std::string line = std::string("[Info] ") + info;
        std::cerr << line << std::endl;
        _lines.push_back(std::make_pair(LogLevel::Info, std::move(line)));
    }

    void Logger::LogSuccess(const char* success)
    {
        std::string line = std::string("[Success] ") + success;
        std::cerr << line << std::endl;
        _lines.push_back(std::make_pair(LogLevel::Success, std::move(line)));
    }

    void Logger::LogWarning(const char* warning)
    {
        std::string line = std::string("[Warning] ") + warning;
        std::cerr << line << std::endl;
        _lines.push_back(std::make_pair(LogLevel::Warning, std::move(line)));
    }

    void Logger::LogError(const char* error)
    {
        std::string line = std::string("[Error] ") + error;
        std::cerr << line << std::endl;
        _lines.push_back(std::make_pair(LogLevel::Error, std::move(line)));
    }

    void Logger::ClearLog()
    {
        _lines.clear();
    }
}
