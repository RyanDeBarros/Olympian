#pragma once

#include "desc/Fields.h"

namespace oly::editor
{
	struct LoggerEnableDesc
	{
		BoolField debug;
		BoolField info;
		BoolField warning;
		BoolField error;
		BoolField fatal;

		LoggerEnableDesc();
	};

	struct LoggerDesc
	{
		BoolField use_logfile;
		BoolField use_console;
		OptionalIntField<MakeOpt(0), MakeOpt<int>()> max_prior_log_files;
		OptionalIntField<MakeOpt(0), MakeOpt<int>()> max_prior_log_bytes;

		LoggerEnableDesc enable;
		static const detail::Key enable_key;

		LoggerDesc();
	};

	struct ContextDesc
	{
		LoggerDesc logger;
		static const detail::Key logger_key;
	};

	struct ProjectDesc
	{
		ContextDesc context;
		static const detail::Key context_key;
	};
}
