#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <sstream>
#include <fstream>

#include "General.h"

namespace oly
{
	class Logger
	{
		Logger() = default;
		Logger(const Logger&) = delete;
		Logger(Logger&&) = delete;

		std::stringstream stream;
		std::ofstream file;

	public:
		static Logger& instance() { static Logger logger; return logger; }

		struct
		{
			bool console = true;
			bool logfile = true;
		} target;

		enum class Level
		{
			DEBUG,
			INFO,
			WARNING,
			ERROR,
			FATAL
		} level = Level::INFO;
		const Level debug = Level::DEBUG;
		const Level info = Level::INFO;
		const Level warning = Level::WARNING;
		const Level error = Level::ERROR;
		const Level fatal = Level::FATAL;

		struct
		{
			// LATER use macros or config parameters to set debug/info enable in debug/release builds.
			bool debug = true;
			bool info = true;
			bool warning = true;
			bool error = true;
			bool fatal = true;
		} enable;

		struct _timestamp {} timestamp;
		struct _nl {} nl;
		struct _endl {} endl;
		struct _start {} start;
		struct _start_opengl { GLenum code; };
		_start_opengl start_opengl(GLenum code) { return _start_opengl{ code }; }
		struct _start_glfw { int code; };
		_start_glfw start_glfw(int code) { return _start_glfw{ code }; }
		struct _start_prefix { std::string prefix; };
		_start_prefix start_prefix(const char* prefix) { return _start_prefix{ prefix }; }
		_start_prefix start_prefix(std::string&& prefix) { return _start_prefix{ std::move(prefix) }; }

		void set_logfile(const char* filepath, bool append);
		Logger& flush();
		Logger& operator<<(const _timestamp&);
		Logger& operator<<(const _nl&);
		Logger& operator<<(const _endl&);
		Logger& operator<<(const _start&);
		Logger& operator<<(const _start_opengl&);
		Logger& operator<<(const _start_glfw&);
		Logger& operator<<(const _start_prefix&);

		Logger& operator<<(const char*);
		Logger& operator<<(const std::string&);
		template<numeric T>
		Logger& operator<<(T v) { return *this << std::to_string(v); }
	};

	inline Logger& LOG = Logger::instance();
}
