#include "ProjectSelectWindow.h"

#include "core/editor/Editor.h"
#include "core/editor/ProjectInfo.h"
#include "gui/ImGuiWrapper.h"

#include "assets/MetaSplitter.h"
#include "definitions/Keys.h"

#include <ImGuiFileDialog.h>

namespace oly::editor
{
    static const std::string OPEN_PROJECT_FILE = "OpenProjectFolder";

    void ProjectSelectWindow::Open()
    {
        Editor::SetOSWindowSize(920, 690);

        // TODO v9.4 remove once recent projects group is implemented
        _project_file = "D:/Projects/Visual Studio/Olympian/Tester/OlympianTester.oly";
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
        if (imtk::controls::input_text("##ProjectFile", _project_file))
            CheckProjectFile();

        ImGui::SameLine();
        if (ImGui::Button("..."))
        {
            IGFD::FileDialogConfig config;
            config.path = _project_file;
            config.flags = ImGuiFileDialogFlags_Modal;

            ImGuiFileDialog::Instance()->OpenDialog(OPEN_PROJECT_FILE, "Select Project", "Olympian files (*.oly){.oly}", config);
        }

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);

        if (ImGuiFileDialog::Instance()->Display(OPEN_PROJECT_FILE, ImGuiWindowFlags_NoCollapse))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                _project_file = ImGuiFileDialog::Instance()->GetCurrentPath();
                CheckProjectFile();
            }

            ImGuiFileDialog::Instance()->Close();
        }

        if (auto d = imtk::disabled(!_valid_project_file))
        {
            if (ImGui::Button("Open"))
                OpenProject();
        }
    }

    void ProjectSelectWindow::CheckProjectFile()
    {
        std::filesystem::path path = _project_file;
        _valid_project_file = path.is_absolute() && std::filesystem::is_regular_file(path);
        if (_valid_project_file)
        {
            auto meta = detail::MetaSplitter::decode_meta(_project_file.c_str());
            _valid_project_file = _valid_project_file && meta.get_version() == ProjectInfo::GetVersion() && meta.has_type(detail::Key::Meta_Project);
        }
    }

    void ProjectSelectWindow::OpenProject()
    {
        Editor::OpenProject(_project_file);
    }
}
