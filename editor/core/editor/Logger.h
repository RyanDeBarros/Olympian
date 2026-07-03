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
	extern const char* LogLevelPrefix(LogLevel level);

	struct LogEntry
	{
		LogLevel level;
		std::string msg;
	};

	class Logger
	{
		std::vector<LogEntry> _lines;

	public:
		static Logger& Instance();

		static void Log(LogLevel level, std::string msg);
		static void LogInfo(std::string msg);
		static void LogSuccess(std::string msg);
		static void LogWarning(std::string msg);
		static void LogError(std::string msg);
		
		void ClearLog();
		const std::vector<LogEntry>& Lines() const;
	};
}
