#pragma once

#include <sstream>
#include <fstream>
#include <string_view>
#include <source_location>

#include "external/GL.h"
#include "external/GLM.h"
#include "core/types/Meta.h"
#include "core/types/Singleton.h"
#include "core/util/StringParam.h"

namespace oly
{
	namespace internal
	{
		struct LogAccess;
	}

	// TODO v6 string formatting and buffer elements for logger

	// TODO v6 update Editor
	struct LoggerOptions
	{
		bool use_console = true;
		bool use_logfile = true;
		std::optional<size_t> max_prior_log_files = 20;
		std::optional<size_t> max_prior_log_bytes = std::nullopt;
	};

	class Logger final : public Singleton<Logger>
	{
		friend class Singleton<Logger>;

		std::stringstream stream;
		std::ofstream file;

		struct
		{
			bool console = true;
			bool logfile = true;
		} target;

		friend struct internal::LogAccess;
		void start_log(const LoggerOptions& options);
		void end_log();

	public:
		void flush();

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

	public:
		struct
		{
			bool debug = false;
			bool info = true;
			bool warning = true;
			bool error = true;
			bool fatal = true;
		} enable;

		struct _nl {} nl;
		struct _endl {} endl;

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

	namespace internal
	{
		struct LogAccess
		{
			static void start_log(const LoggerOptions& options) { LOG.start_log(options); }
			static void end_log() { LOG.end_log(); }
			static Logger::Impl untagged(bool timestamp = false) { return LOG.untagged(timestamp); }
			static Logger::Impl debug(bool timestamp = false, const char* prefix = nullptr) { return LOG.debug(timestamp, prefix); }
			static Logger::Impl debug(Logger::_opengl g) { return LOG.debug(g); }
			static Logger::Impl debug(Logger::_glfw g) { return LOG.debug(g); }
			static Logger::Impl info(bool timestamp = false, const char* prefix = nullptr) { return LOG.info(timestamp, prefix); }
			static Logger::Impl info(Logger::_opengl g) { return LOG.info(g); }
			static Logger::Impl info(Logger::_glfw g) { return LOG.info(g); }
			static Logger::Impl warning(bool timestamp = false, const char* prefix = nullptr) { return LOG.warning(timestamp, prefix); }
			static Logger::Impl warning(Logger::_opengl g) { return LOG.warning(g); }
			static Logger::Impl warning(Logger::_glfw g) { return LOG.warning(g); }
			static Logger::Impl error(bool timestamp = false, const char* prefix = nullptr) { return LOG.error(timestamp, prefix); }
			static Logger::Impl error(Logger::_opengl g) { return LOG.error(g); }
			static Logger::Impl error(Logger::_glfw g) { return LOG.error(g); }
			static Logger::Impl fatal(bool timestamp = false, const char* prefix = nullptr) { return LOG.fatal(timestamp, prefix); }
			static Logger::Impl fatal(Logger::_opengl g) { return LOG.fatal(g); }
			static Logger::Impl fatal(Logger::_glfw g) { return LOG.fatal(g); }
		};
	}

	extern Logger::Impl operator<<(Logger::Impl, Logger::_nl);
	extern Logger::Impl operator<<(Logger::Impl, Logger::_endl);

	extern Logger::Impl operator<<(Logger::Impl, const void*);
	extern Logger::Impl operator<<(Logger::Impl, const char*);
	extern Logger::Impl operator<<(Logger::Impl, const std::string&);
	extern Logger::Impl operator<<(Logger::Impl, const std::string_view);
	extern Logger::Impl operator<<(Logger::Impl, const StringParam&);
	extern Logger::Impl operator<<(Logger::Impl, bool);
	
	template<numeric T>
	inline Logger::Impl operator<<(Logger::Impl impl, T v) { return impl << std::to_string(v); }
	extern Logger::Impl operator<<(Logger::Impl, glm::vec2);
	extern Logger::Impl operator<<(Logger::Impl, glm::vec3);
	extern Logger::Impl operator<<(Logger::Impl, glm::vec4);
	extern Logger::Impl operator<<(Logger::Impl, glm::mat2);
	extern Logger::Impl operator<<(Logger::Impl, glm::mat3);
}

#define OLY_LOG(...) oly::internal::LogAccess::untagged(__VA_ARGS__)
#define OLY_LOG_DEBUG(...) if (!(oly::LOG.enable.debug)) ; else oly::internal::LogAccess::debug(__VA_ARGS__)
#define OLY_LOG_INFO(...) if (!(oly::LOG.enable.info)) ; else oly::internal::LogAccess::info(__VA_ARGS__)
#define OLY_LOG_WARNING(...) if (!(oly::LOG.enable.warning)) ; else oly::internal::LogAccess::warning(__VA_ARGS__)
#define OLY_LOG_ERROR(...) if (!(oly::LOG.enable.error)) ; else oly::internal::LogAccess::error(__VA_ARGS__)
#define OLY_LOG_FATAL(...) if (!(oly::LOG.enable.fatal)) ; else oly::internal::LogAccess::fatal(__VA_ARGS__)

#define _OLY_ENGINE_LOG_DEBUG(category) OLY_LOG_DEBUG(true, category) << oly::LOG.source_info.full_source()
#define _OLY_ENGINE_LOG_WARNING(category) OLY_LOG_WARNING(true, category) << oly::LOG.source_info.full_source()
#define _OLY_ENGINE_LOG_ERROR(category) OLY_LOG_ERROR(true, category) << oly::LOG.source_info.full_source()
#define _OLY_ENGINE_LOG_FATAL(category) OLY_LOG_FATAL(true, category) << oly::LOG.source_info.full_source()
