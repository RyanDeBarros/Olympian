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
		if (auto _ = imtk::main_menu_bar())
		{
			DrawFileMenu();
			DrawViewMenu();
		}
	}

	void MainMenuBar::DrawFileMenu()
	{
		if (auto _ = imtk::menu("File"))
		{
			if (ImGui::MenuItem("Project Settings"))
				DocumentManager::Instance().Add<ProjectDocument>();
		}
	}

	void MainMenuBar::DrawViewMenu()
	{
		if (auto _ = imtk::menu("View"))
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
		}
	}
}
