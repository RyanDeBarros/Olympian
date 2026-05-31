#pragma once

namespace oly::editor
{
	class Logger
	{
	public:
		static Logger& Instance();

		void LogError(const char* error);
	};
}
