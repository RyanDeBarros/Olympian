#pragma once

#include <stdexcept>
#include <string>

namespace oly
{
	struct CompileError : public std::exception
	{
		CompileError(const char* message = "") : std::exception(message) {}
	};

	enum class ErrorCode
	{
		OLYMPIAN_INIT,
		FILE_IO,
		INDEX_OUT_OF_RANGE,
		GLFW_INIT,
		GLEW_INIT,
		WINDOW_CREATION,
		NULL_POINTER,
		SUBSHADER_COMPILATION,
		SHADER_LINKAGE,
		BAD_EMITTER_PARAMS,
		OTHER
	};

	struct Error : public std::runtime_error
	{
		ErrorCode code;

		Error(ErrorCode code) : std::runtime_error("Olympian Error (" + std::to_string((int)code) + ")"), code(code) {}
		Error(ErrorCode code, const char* message) : std::runtime_error("Olympian Error (" + std::to_string((int)code) + "): " + message), code(code) {}
		Error(ErrorCode code, const std::string& message) : std::runtime_error("Olympian Error (" + std::to_string((int)code) + "): " + message), code(code) {}
	};
}