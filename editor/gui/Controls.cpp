#include "Controls.h"

#include <string>

namespace oly::editor::gui
{
	void FloatControl(const char* label, float& value, const float item_width, const float min, const float max, const char* format, bool logarithmic)
	{
		ImGui::SetNextItemWidth(120.f);
		ImGuiSliderFlags flags = 0;
		if (logarithmic)
			flags |= ImGuiSliderFlags_Logarithmic;
		ImGui::SliderFloat(label, &value, min, max, format, flags);

		static const std::string EDIT_VALUE = "Edit value##";
		const std::string popup = EDIT_VALUE + label;

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			ImGui::OpenPopup(popup.c_str());

		if (ImGui::BeginPopup(popup.c_str()))
		{
			ImGui::SetNextItemWidth(120.f);
			ImGui::InputFloat(label, &value, 0.f, 0.f, format);
			value = std::max(value, min);
			value = std::min(value, max);
			ImGui::EndPopup();
		}
	}
}
