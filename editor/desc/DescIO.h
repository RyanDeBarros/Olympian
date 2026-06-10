#pragma once

#include "desc/OptionalPrimitive.h"

#include "gui/DynamicList.h"
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
		static bool DrawRevertButton();

		template<typename T>
		static bool CheckRevertButton(T& desc, const T& def)
		{
			if (desc != def && DrawRevertButton())
			{
				desc = def;
				return true;
			}
			else
				return false;
		}

	public:
		template<typename T, typename U = T>
		static bool Draw(const char* label, T& data, const T& def, OptionalPrimitive<U> min, OptionalPrimitive<U> max)
		{
			bool dirty = false;
			PrepareValue(label);
			gui::IDScope scope(&data);
			dirty |= gui::InputData<T>{}("", data, min, max);
			dirty |= CheckRevertButton(data, def);
			return dirty;
		}

		template<typename T>
		static bool Draw(const char* label, T& data, const T& def)
		{
			bool dirty = false;
			PrepareValue(label);
			gui::IDScope scope(&data);
			dirty |= gui::InputData<T>{}("", data);
			dirty |= CheckRevertButton(data, def);
			return dirty;
		}
		
		static bool Draw(const char* label, int& data, const int& def, const char** names, size_t count);
		static bool Draw(const char* label, std::string* data, const std::string* def, size_t count);
		static bool Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count);
		static bool Draw(const char* label, std::vector<std::string>& data, const std::vector<std::string>& def, gui::DynamicListState& ui_state);

		template<typename E> requires (std::is_enum_v<E>)
		static bool Draw(const char* label, E& data, const E& def);

	private:
		template<typename E, size_t N>
		static bool DrawEnum(const char* label, E& data, const E& def, const char* const (&values)[N])
		{
			bool dirty = false;
			int index = static_cast<int>(data);
			PrepareValue(label);
			gui::IDScope scope(&data);
			dirty |= ImGui::Combo("", &index, values, N);
			data = static_cast<E>(index);
			dirty |= CheckRevertButton(data, def);
			return dirty;
		}
	};
}
