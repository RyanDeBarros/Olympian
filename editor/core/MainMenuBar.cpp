#include "MainMenuBar.h"

#include "panels/AssetEditorPanel.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/LogPanel.h"
#include "panels/TreeViewPanel.h"

#include <imgui.h>

namespace oly::editor
{
	void MainMenuBar::Init()
	{
		// NOP
	}

	void MainMenuBar::Draw()
	{
		if (ImGui::BeginMainMenuBar())
		{
			DrawWindowMenu();
			ImGui::EndMainMenuBar();
		}
	}

	void MainMenuBar::DrawWindowMenu()
	{
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Asset Editor"))
				AssetEditorPanel::Instance().Open();

			if (ImGui::MenuItem("Content Browser", "Ctrl+SPACE"))
				ContentBrowserPanel::Instance().Open();

			if (ImGui::MenuItem("Log"))
				LogPanel::Instance().Open();

			if (ImGui::MenuItem("Tree View"))
				TreeViewPanel::Instance().Open();

			ImGui::EndMenu();
		}
	}
}
