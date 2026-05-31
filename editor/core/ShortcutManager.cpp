#include "ShortcutManager.h"

#include "core/Editor.h"
#include "core/MainWindow.h"
#include "core/MainMenuBar.h"

namespace oly::editor
{
    ShortcutManager& ShortcutManager::Instance()
    {
        return Editor::Instance().GetShortcutManager();
    }

    void ShortcutManager::PollShortcuts()
    {
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal))
            MainWindow::Instance().GetMainMenuBar().OpenFile();
    }

    void ShortcutManager::HandlePathDrop(int count, const char** paths)
    {
        for (int i = 0; i < count; ++i)
            Editor::Instance().OpenFile(paths[i]);
    }
}
