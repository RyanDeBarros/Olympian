#include "ShortcutManager.h"

#include "core/editor/Editor.h"
#include "core/windows/MainWindow.h"

#include "panels/AssetEditorPanel.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/PreferencesPanel.h"

namespace oly::editor
{
    ShortcutManager& ShortcutManager::Instance()
    {
        return Editor::Instance().GetShortcutManager();
    }

    static void PollGeneralShortcuts()
    {
        if (ImGui::Shortcut(ImGuiKey_F11, ImGuiInputFlags_RouteGlobal))
            Editor::Instance().SetOSWindowFullScreen(!Editor::Instance().IsOSWindowFullScreen());
    }

    static void PollMainAppStateShortcuts()
    {
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Space, ImGuiInputFlags_RouteGlobal))
            ContentBrowserPanel::Instance().ToggleOpen();

        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Comma, ImGuiInputFlags_RouteGlobal))
            PreferencesPanel::Instance().Open();
    }

    void ShortcutManager::PollShortcuts()
    {
        PollGeneralShortcuts();
        if (Editor::Instance().GetAppState() == AppState::Main)
            PollMainAppStateShortcuts();
    }

    void ShortcutManager::HandlePathDrop(int count, const char** paths)
    {
        for (int i = 0; i < count; ++i)
            Editor::Instance().OpenFile(paths[i]);
    }
}
