#pragma once

#include <string>
#include <vector>

#include <imgui.h>

namespace oly::editor
{
	enum class LogLevel
	{
		Info,
		Success,
		Warning,
		Error
	};

	extern ImU32 LogLevelColor(LogLevel level);

	class Logger
	{
		std::vector<std::pair<LogLevel, std::string>> _lines;

	public:
		static Logger& Instance();

		void Log(LogLevel level, const char* msg);
		void LogInfo(const char* info);
		void LogSuccess(const char* success);
		void LogWarning(const char* warning);
		void LogError(const char* error);
		
		void ClearLog();
	};
}
