#pragma once

#include "desc/OptionalPrimitive.h"

#include "external/GL.h"
#include "external/GLM.h"

#include <string>

namespace oly::editor
{
	struct DescIO
	{
		static bool Draw(const char* label, bool& data, const bool* disk);
		static bool Draw(const char* label, int& data, const int* disk, OptionalInt min, OptionalInt max);
		static bool Draw(const char* label, float& data, const float* disk, OptionalFloat min, OptionalFloat max);
		static bool Draw(const char* label, OptionalInt& data, const OptionalInt* disk, OptionalInt min, OptionalInt max);
		static bool Draw(const char* label, OptionalFloat& data, const OptionalFloat* disk, OptionalFloat min, OptionalFloat max);
		static bool Draw(const char* label, int& data, const int* disk, const char** names, size_t count);
		static bool Draw(const char* label, std::string& data, const std::string* disk);
		static bool DrawColor(const char* label, glm::vec4& data, const glm::vec4* disk);

		template<typename E> requires (std::is_enum_v<E>)
		static bool Draw(const char* label, E& data, const E* disk);
	};
}
