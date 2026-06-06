#pragma once

#include <string>
#include <vector>

#include <imgui.h>

namespace oly::editor::gui
{
	extern bool Combo(const char* label, int& current_item, std::vector<std::string>& items);

	extern bool InputText(const char* label, std::string& string, size_t max_size = 256,
		ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
}
