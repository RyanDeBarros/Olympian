#pragma once

#include <sstream>
#include <fstream>
#include <string_view>
#include <source_location>

#include "external/GL.h"
#include "external/GLM.h"
#include "core/types/Meta.h"

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

	private:
		void pass_timestamp();

	public:
		class Impl
		{
			bool enabled;

			friend class Logger;
			Impl(bool enabled) : enabled(enabled) {}

		public:
			template<typename T>
			Impl stream(T&& obj) { if (enabled) Logger::instance().stream << std::forward<T>(obj); return *this; }
		};
		friend class Impl;

	private:
		struct _opengl { GLenum code; };
		struct _glfw { int code; };
		
	public:
		_opengl opengl(GLenum code) { return _opengl{ code }; }
		_glfw glfw(int code) { return _glfw{ code }; }

	private:
		void start(const char* level, bool timestamp, const char* prefix);
		void start(const char* level, _opengl);
		void start(const char* level, _glfw);

	public:
		Impl untagged(bool timestamp = false);
		Impl debug(bool timestamp = false, const char* prefix = nullptr);
		Impl debug(_opengl);
		Impl debug(_glfw);
		Impl info(bool timestamp = false, const char* prefix = nullptr);
		Impl info(_opengl);
		Impl info(_glfw);
		Impl warning(bool timestamp = false, const char* prefix = nullptr);
		Impl warning(_opengl);
		Impl warning(_glfw);
		Impl error(bool timestamp = false, const char* prefix = nullptr);
		Impl error(_opengl);
		Impl error(_glfw);
		Impl fatal(bool timestamp = false, const char* prefix = nullptr);
		Impl fatal(_opengl);
		Impl fatal(_glfw);

		struct
		{
			// TODO v3 add these to project config.
			bool debug = true;
			bool info = true;
			bool warning = true;
			bool error = true;
			bool fatal = true;
		} enable;

		struct _nl {} nl;
		struct _endl {} endl;

		void set_logfile(const char* filepath, bool append);
		void flush();

		struct SourceInfo
		{
			std::string file_name(std::source_location location = std::source_location::current()) const;
			std::string line(std::source_location location = std::source_location::current()) const;
			std::string column(std::source_location location = std::source_location::current()) const;
			std::string function_name(std::source_location location = std::source_location::current()) const;

			std::string full_source(std::source_location location = std::source_location::current()) const;
		} source_info;
	};

	inline Logger& LOG = Logger::instance();

	extern Logger::Impl operator<<(Logger::Impl, Logger::_nl);
	extern Logger::Impl operator<<(Logger::Impl, Logger::_endl);

	extern Logger::Impl operator<<(Logger::Impl, const void*);
	extern Logger::Impl operator<<(Logger::Impl, const char*);
	extern Logger::Impl operator<<(Logger::Impl, const std::string&);
	extern Logger::Impl operator<<(Logger::Impl, std::string_view);
	extern Logger::Impl operator<<(Logger::Impl, bool);
	
	template<numeric T>
	inline Logger::Impl operator<<(Logger::Impl impl, T v) { return impl << std::to_string(v); }
	extern Logger::Impl operator<<(Logger::Impl, glm::vec2);
	extern Logger::Impl operator<<(Logger::Impl, glm::vec3);
	extern Logger::Impl operator<<(Logger::Impl, glm::vec4);
}

// TODO v3 use these macros and make regular debug/info/etc. methods private. These implement short-circuiting
#define OLY_LOG_DEBUG(...) if (!(LOG.enable.debug)) ; else LOG.debug(__VA_ARGS__)
#define OLY_LOG_INFO(...) if (!(LOG.enable.info)) ; else LOG.info(__VA_ARGS__)
#define OLY_LOG_WARNING(...) if (!(LOG.enable.warning)) ; else LOG.warning(__VA_ARGS__)
#define OLY_LOG_ERROR(...) if (!(LOG.enable.error)) ; else LOG.error(__VA_ARGS__)
#define OLY_LOG_FATAL(...) if (!(LOG.enable.fatal)) ; else LOG.fatal(__VA_ARGS__)
