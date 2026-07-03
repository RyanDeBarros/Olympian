#pragma once

#include <string>

namespace oly::editor
{
	enum class LogLevel : int;

	struct Notification
	{
		LogLevel level;
		std::string message;
		float timer;
		float age = 0.f;

		Notification(LogLevel level, std::string message, float timer = 3.f);
	};

	struct Notifier
	{
		static void Notify(LogLevel level, std::string message, float timer = 3.f);
		static void NotifyInfo(std::string message, float timer = 3.f);
		static void NotifySuccess(std::string message, float timer = 3.f);
		static void NotifyWarning(std::string message, float timer = 3.f);
		static void NotifyError(std::string message, float timer = 3.f);
	};
}
