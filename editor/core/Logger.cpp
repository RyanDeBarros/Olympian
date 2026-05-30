#include "Logger.h"

#include <iostream>

void Logger::LogError(const char* error)
{
    // TODO v7 editor log
    std::cerr << "[Error]" << error << std::endl;
}
