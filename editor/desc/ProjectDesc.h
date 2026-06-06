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

		void Reset(LoggerEnableDesc& source);
		void Isolate();
	};

#define LOGGER_ENABLE_GENERATOR(M) \
	M(debug) \
	M(info) \
	M(warning) \
	M(error) \
	M(fatal)

	struct LoggerDesc
	{
		BoolField use_logfile;
		BoolField use_console;
		OptionalIntField<MakeOpt(0), MakeOpt<int>()> max_prior_log_files;
		OptionalIntField<MakeOpt(0), MakeOpt<int>()> max_prior_log_bytes;

		LoggerEnableDesc enable;
		static const detail::Key enable_key;

		LoggerDesc();

		void Reset(LoggerDesc& source);
		void Isolate();
	};

#define LOGGER_PARTIAL_GENERATOR(M) \
	M(use_logfile) \
	M(use_console) \
	M(max_prior_log_files) \
	M(max_prior_log_bytes)

#define LOGGER_GENERATOR(M) \
	LOGGER_PARTIAL_GENERATOR(M) \
	M(enable)

	struct ContextDesc
	{
		LoggerDesc logger;
		static const detail::Key logger_key;

		void Reset(ContextDesc& source);
		void Isolate();
	};

#define CONTEXT_GENERATOR(M) \
	M(logger)

	struct ProjectDesc
	{
		ContextDesc context;
		static const detail::Key context_key;

		void Reset(ProjectDesc& source);
		void Isolate();
	};

#define PROJECT_GENERATOR(M) \
	M(context)
}
