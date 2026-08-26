#include "Errors.h"

#include <stack>

#include <imtk.hpp>

namespace oly::editor
{
	static std::stack<bool> NOTIFY_STACK;

	BreakoutError::BreakoutError(const char* message)
		: std::exception(message)
	{
	}

	void BreakoutError::Throw(std::string_view message)
	{
		Log(message.data());
		throw BreakoutError(message.data());
	}

	void BreakoutError::Log(std::string_view error)
	{
		if (!NOTIFY_STACK.empty() && NOTIFY_STACK.top())
			imtk::notify_error(std::string(error));
		else
			imtk::log_error(std::string(error));
	}

	BreakoutError::NotifyScope::NotifyScope(bool notify)
	{
		NOTIFY_STACK.push(notify);
	}

	BreakoutError::NotifyScope::~NotifyScope()
	{
		NOTIFY_STACK.pop();
	}
}
