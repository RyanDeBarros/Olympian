#include "Logger.h"

#include "core/Editor.h"

#include <iostream>

namespace oly::editor
{
    Logger& Logger::Instance()
    {
        return Editor::Instance().GetLogger();
    }

    void Logger::LogError(const char* error)
    {
        // TODO v7 editor log
        std::cerr << "[Error]" << error << std::endl;
    }
}
