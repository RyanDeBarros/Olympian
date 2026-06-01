#pragma once

#include "definitions/Enums.h"

#include "GL.h"

#include <optional>

namespace oly::editor
{
	struct DescIO
	{
		static bool Draw(const char* label, bool& data);
		static bool Draw(const char* label, int& data, std::optional<int> min, std::optional<int> max);
		static bool Draw(const char* label, detail::StorageMode& data);
		static bool Draw(const char* label, detail::SVGMipmapGenerationMode& data);
		static bool Draw(const char* label, GLenum& data, const GLenum* values, const char** names, size_t count);
	};
}
