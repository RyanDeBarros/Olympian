#include "ContentBrowserPanel.h"

#include <imgui.h>

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
