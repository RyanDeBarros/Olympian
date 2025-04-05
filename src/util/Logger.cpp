#include "Logger.h"

#include <iostream>

namespace oly
{
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
