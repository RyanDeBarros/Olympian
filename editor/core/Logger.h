#pragma once

class Logger
{
public:
	static Logger& Instance();

	void LogError(const char* error);
};
