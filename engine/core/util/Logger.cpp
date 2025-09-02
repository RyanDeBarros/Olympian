#include "Logger.h"

#include "core/util/Time.h"

#include <iostream>
#include <chrono>

// TODO v4 detect size of file when appending and warn when file is getting too large. additional behaviour to create a new file using (i) notation (i.e. olympian.log -> olympian (1).log -> olympian (2).log).
// TODO v4 default behaviour of removing excess initial logs when file is getting too large.

namespace oly
{
	void Logger::pass_timestamp()
	{
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
#pragma warning(suppress : 4996)
		auto current_time = std::localtime(&time);
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		stream << std::put_time(current_time, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
	}

	void Logger::set_logfile(const char* filepath, bool append)
	{
		file.close();
		if (append)
			file.open(filepath, std::ios_base::app);
		else
			file.open(filepath);

		const char* log_start = "--- LOG started at ";
		const char* log_end = " ---";
		auto setw = std::setw(sizeof(log_start) - 1 + 32 + sizeof(log_end) - 1);
		stream << std::setfill('-') << setw << "" << '\n' << log_start;
		pass_timestamp();
		stream << log_end << '\n' << std::setfill('-') << setw << "" << '\n';
		file.flush();
	}

	void Logger::flush()
	{
		if (target.console)
		{
			std::cout << stream.str();
			std::cout.flush();
		}
		if (target.logfile)
		{
			file << stream.str();
			file.flush();
		}
		stream.str(std::string());
		stream.clear();
	}

	void Logger::start(const char* level, bool timestamp, const char* prefix)
	{
		if (timestamp)
		{
			pass_timestamp();
			stream << " ";
		}
		if (prefix)
			stream << "[" << level << " - " << prefix << "] ";
		else
			stream << "[" << level << "] ";
	}

	void Logger::start(const char* level, Logger::_opengl g)
	{
		pass_timestamp();
		stream << " [" << level << " - GL" << g.code << "] ";
	}

	void Logger::start(const char* level, Logger::_glfw g)
	{
		pass_timestamp();
		stream << " [" << level << " - GLFW" << g.code << "] ";
	}

	Logger::Impl Logger::untagged(bool timestamp)
	{
		if (timestamp)
		{
			pass_timestamp();
			stream << " ";
		}
		return Impl(true);
	}

	Logger::Impl Logger::debug(bool timestamp, const char* prefix)
	{
		if (enable.debug)
			start("DEBUG", timestamp, prefix);
		return Impl(enable.debug);
	}

	Logger::Impl Logger::debug(Logger::_opengl g)
	{
		if (enable.debug)
			start("DEBUG", g);
		return Impl(enable.debug);
	}

	Logger::Impl Logger::debug(Logger::_glfw g)
	{
		if (enable.debug)
			start("DEBUG", g);
		return Impl(enable.debug);
	}

	Logger::Impl Logger::info(bool timestamp, const char* prefix)
	{
		if (enable.info)
			start("INFO", timestamp, prefix);
		return Impl(enable.info);
	}

	Logger::Impl Logger::info(Logger::_opengl g)
	{
		if (enable.info)
			start("INFO", g);
		return Impl(enable.info);
	}

	Logger::Impl Logger::info(Logger::_glfw g)
	{
		if (enable.info)
			start("INFO", g);
		return Impl(enable.info);
	}

	Logger::Impl Logger::warning(bool timestamp, const char* prefix)
	{
		if (enable.warning)
			start("WARNING", timestamp, prefix);
		return Impl(enable.warning);
	}

	Logger::Impl Logger::warning(Logger::_opengl g)
	{
		if (enable.warning)
			start("WARNING", g);
		return Impl(enable.warning);
	}

	Logger::Impl Logger::warning(Logger::_glfw g)
	{
		if (enable.warning)
			start("WARNING", g);
		return Impl(enable.warning);
	}

	Logger::Impl Logger::error(bool timestamp, const char* prefix)
	{
		if (enable.error)
			start("ERROR", timestamp, prefix);
		return Impl(enable.error);
	}

	Logger::Impl Logger::error(Logger::_opengl g)
	{
		if (enable.error)
			start("ERROR", g);
		return Impl(enable.error);
	}

	Logger::Impl Logger::error(Logger::_glfw g)
	{
		if (enable.error)
			start("ERROR", g);
		return Impl(enable.error);
	}

	Logger::Impl Logger::fatal(bool timestamp, const char* prefix)
	{
		if (enable.fatal)
			start("FATAL", timestamp, prefix);
		return Impl(enable.fatal);
	}

	Logger::Impl Logger::fatal(Logger::_opengl g)
	{
		if (enable.fatal)
			start("FATAL", g);
		return Impl(enable.fatal);
	}

	Logger::Impl Logger::fatal(Logger::_glfw g)
	{
		if (enable.fatal)
			start("FATAL", g);
		return Impl(enable.fatal);
	}

	std::string Logger::SourceInfo::file_name(std::source_location location) const
	{
		return location.file_name();
	}
	
	std::string Logger::SourceInfo::line(std::source_location location) const
	{
		return std::to_string(location.line());
	}
	
	std::string Logger::SourceInfo::column(std::source_location location) const
	{
		return std::to_string(location.column());
	}
	
	std::string Logger::SourceInfo::function_name(std::source_location location) const
	{
		return location.function_name();
	}
	
	std::string Logger::SourceInfo::full_source(std::source_location location) const
	{
		std::stringstream ss;
		ss << location.file_name() << "(" << location.line() << ":" << location.column() << ") `" << location.function_name() << "`: ";
		return ss.str();
	}

	Logger::Impl operator<<(Logger::Impl impl, Logger::_nl)
	{
		return impl.stream('\n');
	}

	Logger::Impl operator<<(Logger::Impl impl, Logger::_endl)
	{
		impl << LOG.nl;
		LOG.flush();
		return impl;
	}

	Logger::Impl operator<<(Logger::Impl impl, const void* c)
	{
		return impl.stream(c);
	}

	Logger::Impl operator<<(Logger::Impl impl, const char* c)
	{
		return impl.stream(c);
	}

	Logger::Impl operator<<(Logger::Impl impl, const std::string& s)
	{
		return impl.stream(s);
	}

	Logger::Impl operator<<(Logger::Impl impl, std::string_view s)
	{
		return impl.stream(s);
	}

	Logger::Impl operator<<(Logger::Impl impl, bool b)
	{
		return impl << (b ? "true" : "false");
	}

	Logger::Impl operator<<(Logger::Impl impl, glm::vec2 v)
	{
		return impl << "(" << v.x << ", " << v.y << ")";
	}
	
	Logger::Impl operator<<(Logger::Impl impl, glm::vec3 v)
	{
		return impl << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	}
	
	Logger::Impl operator<<(Logger::Impl impl, glm::vec4 v)
	{
		return impl << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
	}
}
