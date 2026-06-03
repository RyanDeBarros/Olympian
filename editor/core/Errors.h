#pragma once

#include <stdexcept>

namespace oly::editor
{
	// Use BreakoutError when an error message has already been logged, and we only need to break out of call stack
	struct BreakoutError : public std::exception
	{
		[[noreturn]] static void Throw(const char* message);
	};
}
