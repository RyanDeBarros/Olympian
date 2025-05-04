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
		TOML_PARSE,
		LOAD_ASSET,
		NSVG_PARSING,
		UNREGISTERED_TEXTURE,
		UNREGISTERED_NSVG_ABSTRACT,
		UNREGISTERED_SPRITE,
		UNREGISTERED_POLYGON,
		UNREGISTERED_ELLIPSE,
		UNREGISTERED_ATLAS,
		UNREGISTERED_DRAW_COMMAND,
		UNREGISTERED_TILESET,
		UNREGISTERED_TILEMAP,
		INCOMPLETE_TILESET,
		UNREGISTERED_PARAGRAPH,
		UNCACHED_GLYPH,
		UNREGISTERED_FONT_FACE,
		UNREGISTERED_FONT_ATLAS,
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