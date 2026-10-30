#include "PreferencesPanel.h"

#include "core/editor/Editor.h"

#include "core/windows/MainWindow.h"

#include "panels/PanelManager.h"

#include "definitions/Keys.h"

#include <imgui.h>

namespace oly::editor
{
    PreferencesPanel::PreferencesPanel()
        : window_unsaved_changes_modal_("Window", { "Editor preferences" })
        , shutdown_unsaved_changes_modal_("Shutdown", { "Editor preferences" })
    {
    }

    PreferencesPanel& PreferencesPanel::Instance()
    {
        if (auto panel = MainWindow::Instance().GetPanelManager().Get<PreferencesPanel>())
            return *panel;
        else
            imtk::breakout_error::throw_("No instance of PreferencesPanel");
    }

    void PreferencesPanel::InitImpl()
    {
        doc_.Init();
    }

    const char* PreferencesPanel::GetTitle() const
    {
        return "Preferences";
    }

    void PreferencesPanel::Draw()
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
        if (doc_.IsDirty())
            flags |= ImGuiWindowFlags_UnsavedDocument;

        auto window = DrawDockedWindow(flags);
        if (window.RequestsClose() && doc_.IsDirty())
        {
            Open();
            ImGui::SetWindowFocus();

            window_unsaved_changes_modal_.pop.open();
        }

        if (window.IsVisible())
        {
            if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
                doc_.DumpAsset();

            if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl, ImGuiInputFlags_RouteGlobal))
                doc_.Undo();

            if (ImGui::Shortcut(ImGuiKey_Z | ImGuiMod_Ctrl | ImGuiMod_Shift, ImGuiInputFlags_RouteGlobal))
                doc_.Redo();

            doc_.DrawMenuBar();
            doc_.Draw();
        }

        if (DrawUnsavedChangesModal(window_unsaved_changes_modal_))
            Close();

        if (DrawUnsavedChangesModal(shutdown_unsaved_changes_modal_))
        {
            Close();
            Editor::RequestShutdown();
        }
    }

    bool PreferencesPanel::RequestShutdown()
    {
        if (doc_.IsDirty())
        {
            Open();
            GainFocus();
            shutdown_unsaved_changes_modal_.pop.open();
            return false;
        }
        else
            return true;
    }

    bool PreferencesPanel::DrawUnsavedChangesModal(imtk::unsaved_changes_modal& modal)
    {
        auto result = modal.draw();

        if (result == imtk::unsaved_changes_modal::result::save_changes)
            doc_.DumpAsset();

        if (result == imtk::unsaved_changes_modal::result::discard_changes)
            doc_.LoadAsset();

        return modal.closing(result);
    }
}
