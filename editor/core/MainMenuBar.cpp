#include "MainMenuBar.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include "core/Editor.h"

namespace oly::editor
{
	static const char* OPEN_FILE = "OpenFileDlg";

	static std::string open_file_parent = ".";

	void MainMenuBar::Draw()
	{
		if (ImGui::BeginMainMenuBar())
		{
			DrawFileMenu();
			ImGui::EndMainMenuBar();
		}

		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize * 0.5f, ImGuiCond_FirstUseEver);

		if (ImGuiFileDialog::Instance()->Display(OPEN_FILE, ImGuiWindowFlags_NoCollapse))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::filesystem::path path = ImGuiFileDialog::Instance()->GetFilePathName();
				open_file_parent = path.parent_path().string();
				Editor::Instance().OpenFile(path);
			}

			ImGuiFileDialog::Instance()->Close();
		}
	}

	void MainMenuBar::DrawFileMenu()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
				OpenFile();

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Open File (Ctrl+O)");

			ImGui::EndMenu();
		}
	}

	void MainMenuBar::OpenFile()
	{
		IGFD::FileDialogConfig config;
		config.path = open_file_parent;
		config.flags = ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering | ImGuiFileDialogFlags_Modal;

		ImGuiFileDialog::Instance()->OpenDialog(OPEN_FILE, "Select File", "Olympian files (*.oly){.oly},Image files (*.png, *.jpg, *.gif){.png,.jpg,.gif},Font files (*.ttf, *.otf){.ttf,.otf},Any files{.*}", config);
	}
}
