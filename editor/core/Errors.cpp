#include "Errors.h"

#include "core/editor/Logger.h"

namespace oly::editor
{
	BreakoutError::BreakoutError(const char* message)
		: std::exception(message)
	{
	}

	void BreakoutError::Throw(const char* message)
	{
		Logger::Instance().Log(LogLevel::Error, message);
		throw BreakoutError(message);
	}
}
