#include "UnsavedChangesModal.h"

#include <imgui.h>

namespace oly::editor::gui
{
	UnsavedChangesModalResult DrawUnsavedChangesModal(const char* popup, std::vector<std::string>& description)
	{
		UnsavedChangesModalResult result = UnsavedChangesModalResult::None;

		if (ImGui::BeginPopupModal(popup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			for (const auto& line : description)
				ImGui::TextUnformatted(line.c_str());

			if (ImGui::Button("Save Changes"))
			{
				ImGui::CloseCurrentPopup();
				result = UnsavedChangesModalResult::SaveChanges;
			}

			ImGui::SameLine();
			if (ImGui::Button("Discard Changes"))
			{
				ImGui::CloseCurrentPopup();
				result = UnsavedChangesModalResult::DiscardChanges;
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel Close"))
			{
				ImGui::CloseCurrentPopup();
				result = UnsavedChangesModalResult::CancelClose;
			}

			ImGui::EndPopup();
		}

		return result;
	}
}
