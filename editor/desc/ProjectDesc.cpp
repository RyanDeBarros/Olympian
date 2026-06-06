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

	void LoggerEnableDesc::Reset(LoggerEnableDesc& source)
	{
		RESET_FIELDS(LOGGER_ENABLE_GENERATOR);
	}

	void LoggerEnableDesc::Isolate()
	{
		ISOLATE_FIELDS(LOGGER_ENABLE_GENERATOR);
	}

	const detail::Key LoggerDesc::enable_key = detail::Key::Enable;

	LoggerDesc::LoggerDesc()
		: use_logfile(true, detail::Key::UseLogfile, "Use Logfile"),
		use_console(true, detail::Key::UseConsole, "Use Console"),
		max_prior_log_files(MakeOpt<int>(), detail::Key::MaxPriorLogFiles, detail::Key::EnableMaxPriorLogFiles, "Max Prior Log Files"),
		max_prior_log_bytes(MakeOpt<int>(), detail::Key::MaxPriorLogBytes, detail::Key::EnableMaxPriorLogBytes, "Max Prior Log Bytes"),
		enable()
	{
	}

	void LoggerDesc::Reset(LoggerDesc& source)
	{
		RESET_FIELDS(LOGGER_GENERATOR);
	}

	void LoggerDesc::Isolate()
	{
		ISOLATE_FIELDS(LOGGER_GENERATOR);
	}

	const detail::Key ContextDesc::logger_key = detail::Key::Logger;

	void ContextDesc::Reset(ContextDesc& source)
	{
		RESET_FIELDS(CONTEXT_GENERATOR);
	}

	void ContextDesc::Isolate()
	{
		ISOLATE_FIELDS(CONTEXT_GENERATOR);
	}

	const detail::Key ProjectDesc::context_key = detail::Key::Context;

	void ProjectDesc::Reset(ProjectDesc& source)
	{
		RESET_FIELDS(PROJECT_GENERATOR);
	}

	void ProjectDesc::Isolate()
	{
		ISOLATE_FIELDS(PROJECT_GENERATOR);
	}
}
