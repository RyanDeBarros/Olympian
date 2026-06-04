#include "ShortcutManager.h"

#include "core/Editor.h"
#include "core/MainWindow.h"

#include "panels/AssetEditorPanel.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/PreferencesPanel.h"

namespace oly::editor
{
    ShortcutManager& ShortcutManager::Instance()
    {
        return Editor::Instance().GetShortcutManager();
    }

    void ShortcutManager::PollShortcuts()
    {
        if (ImGui::Shortcut(ImGuiKey_F11, ImGuiInputFlags_RouteGlobal))
            Editor::Instance().SetOSWindowFullScreen(!Editor::Instance().IsOSWindowFullScreen());
        
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Space, ImGuiInputFlags_RouteGlobal))
        {
            auto& panel = ContentBrowserPanel::Instance();
            if (panel.IsOpen())
                panel.Close();
            else
                panel.Open();
        }

        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Comma, ImGuiInputFlags_RouteGlobal))
            PreferencesPanel::Instance().Open();
    }

    void ShortcutManager::HandlePathDrop(int count, const char** paths)
    {
        for (int i = 0; i < count; ++i)
            Editor::Instance().OpenFile(paths[i]);
    }
}
