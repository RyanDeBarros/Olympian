#include "Logger.h"

#include "Editor.h"

#include <iostream>

Logger& Logger::Instance()
{
    return Editor::Instance().GetLogger();
}

void Logger::LogError(const char* error)
{
    // TODO v7 editor log
    std::cerr << "[Error]" << error << std::endl;
}
