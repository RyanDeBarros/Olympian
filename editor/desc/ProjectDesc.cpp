#include "ProjectDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	LoggerEnableDesc::LoggerEnableDesc()
		: debug(false, detail::Key::Debug, "Debug"),
		info(true, detail::Key::Info, "Info"),
		warning(true, detail::Key::Warning, "Warning"),
		error(true, detail::Key::Error, "Error"),
		fatal(true, detail::Key::Fatal, "Fatal")
	{
	}

	const detail::Key LoggerDesc::enable_key = detail::Key::Enable;

	LoggerDesc::LoggerDesc()
		: use_logfile(true, detail::Key::UseLogfile, "Use Logfile"),
		use_console(true, detail::Key::UseConsole, "Use Console"),
		max_prior_log_files(MakeOpt<int>(), detail::Key::MaxPriorLogFiles, detail::Key::EnableMaxPriorLogFiles, "Maxa Prior Log Files"),
		max_prior_log_bytes(MakeOpt<int>(), detail::Key::MaxPriorLogBytes, detail::Key::EnableMaxPriorLogBytes, "Maxa Prior Log Bytes"),
		enable()
	{
	}
}
