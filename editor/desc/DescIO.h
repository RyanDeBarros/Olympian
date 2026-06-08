#pragma once

#include "desc/OptionalPrimitive.h"

#include "gui/IDScope.h"
#include "gui/ImGuiWrapper.h"

#include "external/GL.h"
#include "external/GLM.h"

#include <string>

namespace oly::editor
{
	struct DescIO
	{
	private:
		static void PrepareValue(const char* label);
		static bool DrawRevertButton(const void* ptr_id);

		template<typename T>
		static bool CheckRevertButton(T& desc, const T* disk)
		{
			if (disk && desc != *disk && DrawRevertButton(disk))
			{
				desc = *disk;
				return true;
			}
			else
				return false;
		}

	public:
		template<typename T, typename U = T>
		static bool Draw(const char* label, T& data, const T* disk, OptionalPrimitive<U> min, OptionalPrimitive<U> max)
		{
			bool dirty = false;
			PrepareValue(label);
			gui::IDScope scope(&data);
			dirty |= gui::InputData<T>{}("", data, min, max);
			dirty |= CheckRevertButton(data, disk);
			return dirty;
		}

		template<typename T>
		static bool Draw(const char* label, T& data, const T* disk)
		{
			bool dirty = false;
			PrepareValue(label);
			gui::IDScope scope(&data);
			dirty |= gui::InputData<T>{}("", data);
			dirty |= CheckRevertButton(data, disk);
			return dirty;
		}
		
		static bool Draw(const char* label, int& data, const int* disk, const char** names, size_t count);
		static bool Draw(const char* label, std::string* data, const std::string* disk, size_t count);
		static bool Draw(const char* label, bool* data, const bool* disk, const char** sublabels, size_t count);

		template<typename E> requires (std::is_enum_v<E>)
		static bool Draw(const char* label, E& data, const E* disk);

	private:
		template<typename E, size_t N>
		static bool DrawEnum(const char* label, E& data, const E* disk, const char* const (&values)[N])
		{
			bool dirty = false;
			int index = static_cast<int>(data);
			PrepareValue(label);
			gui::IDScope scope(&data);
			dirty |= ImGui::Combo("", &index, values, N);
			data = static_cast<E>(index);
			dirty |= CheckRevertButton(data, disk);
			return dirty;
		}
	};
}
