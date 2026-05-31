#include "MainMenuBar.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

static const char* OPEN_FILE = "OpenFileDlg";

void MainMenuBar::Draw()
{
	if (ImGui::BeginMainMenuBar())
	{
		DrawFileMenu();
		ImGui::EndMainMenuBar();
	}

	if (ImGuiFileDialog::Instance()->Display(OPEN_FILE))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
			// TODO v7 open path -> send to MainWindow()::Instance(), not asset editor panel or document manager directly
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void MainMenuBar::DrawFileMenu()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Open"))
		{
			IGFD::FileDialogConfig config;
			config.path = "."; // TODO v7 persist path
			ImGuiFileDialog::Instance()->OpenDialog(OPEN_FILE, "Open File", ".oly,.png,.jpg,.gif,.ttf,.otf,.*", config);
			// TODO v7 set next window min size (without cache, dialog opens with 0x0 size)
		}
	
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Open File (Ctrl+O)"); // TODO v7 hook up shortcuts

		ImGui::EndMenu();
	}
}
