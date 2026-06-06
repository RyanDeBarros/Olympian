#include "ImGuiWrapper.h"

namespace oly::editor::gui
{
	static const char* StringVectorComboGetter(void* data, int idx)
	{
		auto& items = *static_cast<std::vector<std::string>*>(data);
		if (idx < 0 || idx >= items.size())
			return nullptr;
		else
			return items[idx].c_str();
	}

	bool Combo(const char* label, int& current_item, std::vector<std::string>& items)
	{
		return ImGui::Combo(label, &current_item, &StringVectorComboGetter, &items, static_cast<int>(items.size()));
	}

	bool InputText(const char* label, std::string& string, size_t max_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		std::string buf;
		buf.resize(max_size);
		string.copy(buf.data(), buf.size());
		bool changed = ImGui::InputText(label, buf.data(), buf.size(), flags, callback, user_data);
		string = std::move(buf);
		return changed;
	}
}
