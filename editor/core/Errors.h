#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

// TODO v9.3 move BreakoutError to imtk

namespace oly::editor
{
	struct BreakoutError : public std::exception
	{
		BreakoutError(const char* message);

		[[noreturn]] static void Throw(std::string_view message);

		static void Log(std::string_view error);

		struct NotifyScope
		{
			NotifyScope(bool notify);
			NotifyScope(const NotifyScope&) = delete;
			NotifyScope(NotifyScope&&) = delete;
			~NotifyScope();
		};
	};
}
