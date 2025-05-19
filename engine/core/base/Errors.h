#pragma once

#include <stdexcept>
#include <string>

namespace oly
{
	enum class ErrorCode
	{
		OLYMPIAN_INIT,
		FILE_IO,
		INDEX_OUT_OF_RANGE,
		UTF,
		GLFW_INIT,
		GLEW_INIT,
		CONTEXT_INIT,
		PLATFORM_INIT,
		WINDOW_CREATION,
		NULL_POINTER,
		INVALID_ITERATOR,
		SUBSHADER_COMPILATION,
		SHADER_LINKAGE,
		INACCESSIBLE,
		WAIT_FAILED,
		OUT_OF_TIME,
		INCOMPATIBLE_TRANSFORMER_MODIFIER,
		INCOMPATIBLE_SIGNAL_TYPE,
		INVALID_CONTROLLER_ID,
		IMAGE_LOAD,
		TOML_PARSE,
		LOAD_ASSET,
		NSVG_PARSING,
		UNREGISTERED_TEXTURE,
		UNREGISTERED_NSVG_ABSTRACT,
		INCOMPLETE_TILESET,
		UNCACHED_GLYPH,
		BAD_COLLISION_PRIMITIVE,
		TRIANGULATION,
		SOLVER_NO_SOLUTION,
		SOLVER_INFINITE_SOLUTIONS,
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