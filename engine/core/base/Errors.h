#pragma once

#include <stdexcept>
#include <string>

namespace oly
{
	enum class ErrorCode
	{
		OlympianInit,
		FileIO,
		IndexOutOfRange,
		Utf,
		GlfwInit,
		GlewInit,
		ContextInit,
		PlatformInit,
		WindowCreation,
		NullPointer,
		UnreachableCode,
		NullStorage,
		StorageOverflow,
		EmptyDataStructure,
		UnsupportedSwitchCase,
		InvalidIterator,
		ConditionFailed,
		InvalidType,
		InvalidID,
		BadCast,
		BadVariant,
		BadMutability,
		CannotParse,
		NotImplemented,
		InvalidSize,
		DoesNotExist,
		DuplicateKey,
		CircularReference,
		BadReference,
		InconsistentData,
		LockedResource,
		SubshaderCompilation,
		ShaderLinkage,
		Inaccessible,
		WaitFailed,
		OutOfTime,
		IncompatibleTransformModifier,
		IncompatibleSignalType,
		InvalidControllerID,
		LoadImage,
		LoadFont,
		TomlParse,
		LoadAsset,
		LoadEnum,
		NsvgParsing,
		UnregisteredTexture,
		UnregisteredNsvgAbstract,
		IncompleteTileset,
		UncachedGlyph,
		Triangulation,
		GjkOverflow,
		EpaOverflow,
		BadCollisionShape,
		SolverNoSolution,
		SolverInfiniteSolutions,
		Other
	};

	struct Error : public std::runtime_error
	{
		ErrorCode code;

		Error(ErrorCode code) : std::runtime_error("Olympian Error (" + std::to_string((int)code) + ")"), code(code) {}
		Error(ErrorCode code, const std::string_view message) : std::runtime_error("Olympian Error (" + std::to_string((int)code) + "): " + std::string(message)), code(code) {}
	};
}