#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace oly::editor
{
	struct BreakoutError : public std::exception
	{
		BreakoutError(const char* message);

		[[noreturn]] static void Throw(std::string_view message);

		static void Log(const char* error);

		struct NotifyScope
		{
			NotifyScope(bool notify);
			NotifyScope(const NotifyScope&) = delete;
			NotifyScope(NotifyScope&&) = delete;
			~NotifyScope();
		};
	};
}
