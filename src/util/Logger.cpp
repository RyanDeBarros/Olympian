#include "Logger.h"

#include <iostream>
#include <chrono>

// LATER detect size of file when appending and warn when file is getting too large. additional behaviour to create a new file using (i) notation (i.e. olympian.log -> olympian (1).log -> olympian (2).log).
// LATER default behaviour of removing excess initial logs when file is getting too large.

namespace oly
{
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
		*this << timestamp;
		stream << log_end << '\n' << std::setfill('-') << setw << "" << '\n';
		file.flush();
	}

	Logger& Logger::flush()
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
		return *this;
	}

	Logger& Logger::operator<<(const _timestamp&)
	{
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
#pragma warning(suppress : 4996)
		auto current_time = std::localtime(&time);
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		stream << std::put_time(current_time, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
		return *this;
	}

	Logger& Logger::operator<<(const _nl&)
	{
		return *this << "\n";
	}

	Logger& Logger::operator<<(const _endl&)
	{
		return (*this << nl).flush();
	}

	Logger& Logger::operator<<(const _start&)
	{
		switch (level)
		{
		case Level::DEBUG:
			if (enable.debug)
				return *this << "[DEBUG] ";
			break;
		case Level::INFO:
			if (enable.info)
				return *this << "[INFO] ";
			break;
		case Level::WARNING:
			if (enable.warning)
				return *this << "[WARNING] ";
			break;
		case Level::ERROR:
			if (enable.error)
				return *this << "[ERROR] ";
			break;
		case Level::FATAL:
			if (enable.fatal)
				return *this << "[FATAL] ";
			break;
		}
		return *this;
	}

	Logger& Logger::operator<<(const _start_opengl& start_gl)
	{
		switch (level)
		{
		case Level::DEBUG:
			if (enable.debug)
				return *this << "[DEBUG - GL" << start_gl.code << "] ";
			break;
		case Level::INFO:
			if (enable.info)
				return *this << "[INFO - GL" << start_gl.code << "] ";
			break;
		case Level::WARNING:
			if (enable.warning)
				return *this << "[WARNING - GL" << start_gl.code << "] ";
			break;
		case Level::ERROR:
			if (enable.error)
				return *this << "[ERROR - GL" << start_gl.code << "] ";
			break;
		case Level::FATAL:
			if (enable.fatal)
				return *this << "[FATAL - GL" << start_gl.code << "] ";
			break;
		}
		return *this;
	}

	Logger& Logger::operator<<(const _start_glfw& start_glfw)
	{
		switch (level)
		{
		case Level::DEBUG:
			if (enable.debug)
				return *this << "[DEBUG - GLFW" << start_glfw.code << "] ";
			break;
		case Level::INFO:
			if (enable.info)
				return *this << "[INFO - GLFW" << start_glfw.code << "] ";
			break;
		case Level::WARNING:
			if (enable.warning)
				return *this << "[WARNING - GLFW" << start_glfw.code << "] ";
			break;
		case Level::ERROR:
			if (enable.error)
				return *this << "[ERROR - GLFW" << start_glfw.code << "] ";
			break;
		case Level::FATAL:
			if (enable.fatal)
				return *this << "[FATAL - GLFW" << start_glfw.code << "] ";
			break;
		}
		return *this;
	}

	Logger& Logger::operator<<(const _start_prefix& start_prefix)
	{
		switch (level)
		{
		case Level::DEBUG:
			if (enable.debug)
				return *this << "[DEBUG - " << start_prefix.prefix << "] ";
			break;
		case Level::INFO:
			if (enable.info)
				return *this << "[INFO - " << start_prefix.prefix << "] ";
			break;
		case Level::WARNING:
			if (enable.warning)
				return *this << "[WARNING - " << start_prefix.prefix << "] ";
			break;
		case Level::ERROR:
			if (enable.error)
				return *this << "[ERROR - " << start_prefix.prefix << "] ";
			break;
		case Level::FATAL:
			if (enable.fatal)
				return *this << "[FATAL - " << start_prefix.prefix << "] ";
			break;
		}
		return *this;
	}

	Logger& Logger::operator<<(const char* c)
	{
		stream << c;
		return *this;
	}

	Logger& Logger::operator<<(const std::string& s)
	{
		stream << s;
		return *this;
	}
}
