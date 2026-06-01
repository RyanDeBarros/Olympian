#pragma once

#include "desc/OptionalPrimitive.h"

#include "GL.h"
#include "TOML.h"

#include "assets/TranslateKey.h"

namespace oly::editor
{
	struct DescIO
	{
		static bool BeginForm(void* id);
		static void EndForm();

		static bool Draw(const char* label, bool& data, const bool* disk);
		static bool Draw(const char* label, int& data, const int* disk, std::optional<int> min, std::optional<int> max);
		static bool Draw(const char* label, float& data, const float* disk, std::optional<float> min, std::optional<float> max);
		static bool Draw(const char* label, int& data, const int* disk, const char** names, size_t count);

		template<typename E> requires (std::is_enum_v<E>)
		static bool Draw(const char* label, E& data, const E* disk);
	};
}
