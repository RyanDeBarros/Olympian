#include "Errors.h"

#include "core/editor/Logger.h"
#include "core/windows/MainWindow.h"

#include <stack>

namespace oly::editor
{
	static std::stack<bool> NOTIFY_STACK;

	BreakoutError::BreakoutError(const char* message)
		: std::exception(message)
	{
	}

	void BreakoutError::Throw(const char* message)
	{
		if (!NOTIFY_STACK.empty() && NOTIFY_STACK.top())
			MainWindow::Instance().PushNotification(Notification(LogLevel::Error, message));
		else
			Logger::Instance().Log(LogLevel::Error, message);

		throw BreakoutError(message);
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
