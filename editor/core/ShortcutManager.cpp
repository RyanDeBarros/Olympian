#include "ShortcutManager.h"

#include "core/Editor.h"
#include "core/MainWindow.h"

#include "panels/AssetEditorPanel.h"

namespace oly::editor
{
    ShortcutManager& ShortcutManager::Instance()
    {
        return Editor::Instance().GetShortcutManager();
    }

    void ShortcutManager::PollShortcuts()
    {
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal))
        {
            if (Editor::Instance().GetAppState() == AppState::Main)
                AssetEditorPanel::Instance().OpenFile();
        }

        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
            AssetEditorPanel::Instance().SaveSelectedTab();

        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
            AssetEditorPanel::Instance().SaveAllTabs();

        if (ImGui::Shortcut(ImGuiKey_F11, ImGuiInputFlags_RouteGlobal))
            Editor::Instance().SetOSWindowFullScreen(!Editor::Instance().IsOSWindowFullScreen());
    }

    void ShortcutManager::HandlePathDrop(int count, const char** paths)
    {
        for (int i = 0; i < count; ++i)
            Editor::Instance().OpenFile(paths[i]);
    }
}
