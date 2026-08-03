#include "UnsavedChangesModal.h"

#include <imtk.hpp>

namespace oly::editor::gui
{
	UnsavedChangesModalResult DrawUnsavedChangesModal(const char* popup, std::vector<std::string>& description)
	{
		UnsavedChangesModalResult result = UnsavedChangesModalResult::None;
		imtk::popup pop(popup, false, imtk::window_flags::always_auto_resize());

		if (auto d = pop.draw())
		{
			for (const auto& line : description)
				ImGui::TextUnformatted(line.c_str());

			if (ImGui::Button("Save Changes"))
			{
				d.close();
				result = UnsavedChangesModalResult::SaveChanges;
			}

			ImGui::SameLine();
			if (ImGui::Button("Discard Changes"))
			{
				d.close();
				result = UnsavedChangesModalResult::DiscardChanges;
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel Close"))
			{
				d.close();
				result = UnsavedChangesModalResult::CancelClose;
			}
		}

		return result;
	}
}
