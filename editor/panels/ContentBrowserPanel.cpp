#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace oly::editor
{
	const char* ContentBrowserPanel::GetTitle() const
	{
		return "Content Browser";
	}

	void ContentBrowserPanel::Draw()
	{
		ImGui::Begin(GetTitle(), nullptr, ImGuiWindowFlags_MenuBar);
		// TODO v7
		ImGui::End();
	}
}
