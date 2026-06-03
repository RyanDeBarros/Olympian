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

		void Log(LogLevel level, const char* msg);
		
		void ClearLog();
		const std::vector<LogEntry>& Lines() const;
	};
}
