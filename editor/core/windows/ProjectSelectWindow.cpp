#include "ProjectSelectWindow.h"

#include "core/editor/Editor.h"
#include "core/editor/ProjectInfo.h"

#include "assets/MetaSplitter.h"

#include "definitions/Keys.h"

#include <ImGuiFileDialog.h>

namespace oly::editor
{
    static const std::string kOpenProjectFile = "OpenProjectFolder";

    void ProjectSelectWindow::Open()
    {
        Editor::SetOSWindowSize(920, 690);

        project_file_ = "D:/Projects/Visual Studio/Olympian/Tester/OlympianTester.oly";
        CheckProjectFile();
    }

    void ProjectSelectWindow::Draw()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        auto style_stack = imtk::style_stack().
            push(ImGuiStyleVar_WindowRounding, 0.0f).
            push(ImGuiStyleVar_WindowBorderSize, 0.0f).apply();

        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        if (auto _ = imtk::window("Project Select Window", window_flags))
        {
            style_stack.kill();

            if (auto grid = imtk::prop::grid())
                DrawOpenExistingGroup();
        }
    }

    void ProjectSelectWindow::DrawOpenExistingGroup()
    {
        imtk::group group;

        ImGui::TextUnformatted("Open Existing Project");
        ImGui::TextUnformatted("Project File");

        ImGui::SameLine();
        if (imtk::controls::input_text("##ProjectFile", project_file_))
            CheckProjectFile();

        ImGui::SameLine();
        if (ImGui::Button("..."))
        {
            IGFD::FileDialogConfig config;
            config.path = project_file_;
            config.flags = ImGuiFileDialogFlags_Modal;

            ImGuiFileDialog::Instance()->OpenDialog(
                kOpenProjectFile,
                "Select Project",
                "Olympian files (*.oly){.oly}",
                config);
        }

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);

        if (ImGuiFileDialog::Instance()->Display(kOpenProjectFile, ImGuiWindowFlags_NoCollapse))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                project_file_ = ImGuiFileDialog::Instance()->GetCurrentPath();
                CheckProjectFile();
            }

            ImGuiFileDialog::Instance()->Close();
        }

        if (auto d = imtk::disabled(!valid_project_file_))
        {
            if (ImGui::Button("Open"))
                OpenProject();
        }
    }

    void ProjectSelectWindow::CheckProjectFile()
    {
        std::filesystem::path path = project_file_;
        valid_project_file_ = path.is_absolute() && std::filesystem::is_regular_file(path);
        if (valid_project_file_)
        {
            auto meta = detail::MetaSplitter::decode_meta(project_file_.c_str());
            valid_project_file_ = meta.get_version() == ProjectInfo::GetVersion()
                && meta.has_type(detail::Key::Meta_Project);
        }
    }

    void ProjectSelectWindow::OpenProject()
    {
        Editor::OpenProject(project_file_);
    }
}
