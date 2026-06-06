#pragma once

#include "external/GL.h"

#include "desc/OptionalPrimitive.h"

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

		template<typename E> requires (std::is_enum_v<E>)
		static bool Draw(const char* label, E& data, const E* disk);

		static const char* StringVectorComboGetter(void* data, int idx);
	};
}
