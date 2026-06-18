#include "MainMenuBar.h"

#include "documents/DocumentManager.h"
#include "documents/ProjectDocument.h"

#include "panels/AssetEditorPanel.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/LogPanel.h"
#include "panels/PreferencesPanel.h"
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
			DrawFileMenu();
			DrawViewMenu();
			ImGui::EndMainMenuBar();
		}
	}

	void MainMenuBar::DrawFileMenu()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Project Settings"))
				DocumentManager::Instance().Add<ProjectDocument>();

			ImGui::EndMenu();
		}
	}

	void MainMenuBar::DrawViewMenu()
	{
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Asset Editor"))
				AssetEditorPanel::Instance().Open();

			if (ImGui::MenuItem("Content Browser", "Ctrl+SPACE"))
				ContentBrowserPanel::Instance().Open();

			if (ImGui::MenuItem("Log"))
				LogPanel::Instance().Open();

			if (ImGui::MenuItem("Preferences", "Ctrl+,"))
				PreferencesPanel::Instance().Open();

			if (ImGui::MenuItem("Tree View"))
				TreeViewPanel::Instance().Open();

			ImGui::EndMenu();
		}
	}
}
