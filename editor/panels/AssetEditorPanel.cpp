#include "AssetEditorPanel.h"

#include <imgui.h>

const char* AssetEditorPanel::GetTitle() const
{
	return "Asset Editor";
}

void AssetEditorPanel::Draw()
{
	ImGui::Begin(GetTitle(), nullptr, ImGuiWindowFlags_MenuBar);
	// TODO v7
	ImGui::End();
}
