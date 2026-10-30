#pragma once

#include <string>

namespace oly::editor
{
    class ProjectSelectWindow
    {
        std::string project_file_;
        bool valid_project_file_ = false;

    public:
        void Open();
        void Draw();

    private:
        void DrawOpenExistingGroup();

        void CheckProjectFile();
        void OpenProject();
    };
}
