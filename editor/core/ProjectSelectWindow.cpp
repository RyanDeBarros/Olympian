#include "ProjectSelectWindow.h"

#include "core/Editor.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include <filesystem>

namespace oly::editor
{
    static const char* OPEN_FOLDER = "OpenProjectFolder";

    void ProjectSelectWindow::Open()
    {
        Editor::Instance().SetOSWindowSize(920, 690);

        // TODO v8 store project *file*, not folder
        // TODO v8 remove once recent projects group is implemented
        _project_folder = "D:/Projects/Visual Studio/Olympian/Tester/";
        CheckProjectFolder();
    }

	void ProjectSelectWindow::Draw()
	{
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("Project Select Window", nullptr, window_flags);
        ImGui::PopStyleVar(2);

        DrawOpenExistingGroup();

        ImGui::End();
	}

    void ProjectSelectWindow::DrawOpenExistingGroup()
    {
        ImGui::BeginGroup();
        ImGui::Text("Open Existing Project");

        ImGui::Text("Project Folder");
        ImGui::SameLine();
        if (ImGui::InputText("##ProjectFolder", _project_folder.data(), _project_folder.size()))
            CheckProjectFolder();

        ImGui::SameLine();

        if (ImGui::Button("..."))
        {
            IGFD::FileDialogConfig config;
            config.path = _project_folder;
            config.flags = ImGuiFileDialogFlags_Modal;

            ImGuiFileDialog::Instance()->OpenDialog(OPEN_FOLDER, "Select Folder", nullptr, config);
        }

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);

        if (ImGuiFileDialog::Instance()->Display(OPEN_FOLDER, ImGuiWindowFlags_NoCollapse))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                _project_folder = ImGuiFileDialog::Instance()->GetCurrentPath();
                CheckProjectFolder();
            }

            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::BeginDisabled(!_valid_project_folder);
        if (ImGui::Button("Open"))
            OpenProject();
        ImGui::EndDisabled();

        ImGui::EndGroup();
    }

    void ProjectSelectWindow::CheckProjectFolder()
    {
        std::filesystem::path path = _project_folder;
        _valid_project_folder = path.is_absolute() && std::filesystem::exists(path) && std::filesystem::is_directory(path);
    }

    void ProjectSelectWindow::OpenProject()
    {
        Editor::Instance().OpenProject(_project_folder);
    }
}
