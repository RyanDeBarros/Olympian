#pragma once

#include "desc/OptionalPrimitive.h"

#include "GL.h"
#include "TOML.h"

#include "assets/TranslateKey.h"
#include "definitions/Enums.h"

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

		// TODO v7 put these in a separate file -> change DescIO to desc namespace
		static bool Draw(const char* label, detail::StorageMode& data, const detail::StorageMode* disk);
		static bool Draw(const char* label, detail::SVGMipmapGenerationMode& data, const detail::SVGMipmapGenerationMode* disk);
	};
}
