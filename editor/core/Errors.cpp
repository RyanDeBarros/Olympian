#include "Errors.h"

#include "core/Logger.h"

namespace oly::editor
{
	void BreakoutError::Throw(const char* message)
	{
		Logger::Instance().Log(LogLevel::Error, message);
		throw BreakoutError();
	}
}
