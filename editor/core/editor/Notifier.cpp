#include "Notifier.h"

#include "core/editor/Logger.h"
#include "core/windows/MainWindow.h"

namespace oly::editor
{
	Notification::Notification(LogLevel level, std::string message, float timer)
		: level(level), message(std::move(message)), timer(timer)
	{
	}

	void Notifier::Notify(LogLevel level, std::string message, float timer)
	{
		MainWindow::Instance().PushNotification(Notification(level, std::move(message), timer));
	}

	void Notifier::NotifyInfo(std::string message, float timer)
	{
		Notify(LogLevel::Info, std::move(message), timer);
	}

	void Notifier::NotifySuccess(std::string message, float timer)
	{
		Notify(LogLevel::Success, std::move(message), timer);
	}
	
	void Notifier::NotifyWarning(std::string message, float timer)
	{
		Notify(LogLevel::Warning, std::move(message), timer);
	}
	
	void Notifier::NotifyError(std::string message, float timer)
	{
		Notify(LogLevel::Error, std::move(message), timer);
	}
}
