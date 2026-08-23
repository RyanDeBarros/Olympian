#include "Errors.h"

#include "core/editor/Logger.h"
#include "core/editor/Notifier.h"

#include <stack>

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
			Notifier::NotifyError(std::string(error));
		else
			Logger::LogError(std::string(error));
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
