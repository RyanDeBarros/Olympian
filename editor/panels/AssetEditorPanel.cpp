#include "AssetEditorPanel.h"

#include "core/PathInfo.h"

#include "core/editor/Editor.h"
#include "core/editor/ProjectInfo.h"

#include "core/windows/MainWindow.h"

#include "documents/DocumentManager.h"
#include "documents/IDocument.h"

#include "panels/ContentBrowserPanel.h"
#include "panels/PanelManager.h"

#include "assets/MetaSplitter.h"

#include <ImGuiFileDialog.h>

#include <imp/macros.hpp>

namespace oly::editor
{
    static constexpr const char* kOpenFile = "OpenFileDlg";

    static std::string open_file_parent = ".";

    AssetEditorPanel::AssetEditorPanel()
        : tab_unsaved_changes_modal_("Tab")
        , window_unsaved_changes_modal_("Window")
        , shutdown_unsaved_changes_modal_("Shutdown")
    {
    }

    void AssetEditorPanel::InitImpl()
    {
        open_file_parent = ProjectInfo::Instance().ResourceRoot().generic_string();
    }

    const char* AssetEditorPanel::GetTitle() const
    {
        return "Asset Editor";
    }

    void AssetEditorPanel::Draw()
    {
        auto window = DrawDockedWindow(ImGuiWindowFlags_MenuBar);
        bool draw_window = true;
        if (window.RequestsClose())
        {
            CloseAllTabs(window_unsaved_changes_modal_.pop);
            if (window_unsaved_changes_modal_.pop.is_opening())
            {
                Open();
                ImGui::SetWindowFocus();
            }
            else
                draw_window = false;
        }

        if (draw_window && window.IsVisible())
        {
            PollShortcuts();

            DrawTabBar();

            if (selected_tab_)
                selected_tab_->DrawMenuBar();
            else
                DrawDefaultMenuBar();

            ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize * 0.5f, ImGuiCond_FirstUseEver);

            if (ImGuiFileDialog::Instance()->Display(kOpenFile, ImGuiWindowFlags_NoCollapse))
            {
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    std::filesystem::path path = ImGuiFileDialog::Instance()->GetFilePathName();
                    open_file_parent = path.parent_path().string();
                    Editor::OpenFile(path);
                }

                ImGuiFileDialog::Instance()->Close();
            }
        }

        if (DrawUnsavedChangesModal(window_unsaved_changes_modal_))
            Close();

        if (DrawUnsavedChangesModal(shutdown_unsaved_changes_modal_))
        {
            Close();
            Editor::RequestShutdown();
        }
    }

    AssetEditorPanel& AssetEditorPanel::Instance()
    {
        if (auto panel = MainWindow::Instance().GetPanelManager().Get<AssetEditorPanel>())
            return *panel;
        else
            imtk::breakout_error::throw_("No instance of AssetEditorPanel");
    }

    void AssetEditorPanel::OpenFile()
    {
        IGFD::FileDialogConfig config;
        config.path = open_file_parent;
        config.flags = ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering | ImGuiFileDialogFlags_Modal;

        static const char* const kFilters =
            "Olympian files (*.oly){.oly},"
            "Image files (*.png, *.jpg, *.gif){.png,.jpg,.gif},"
            "Font files (*.ttf, *.otf){.ttf,.otf},"
            "Any files{.*}";

        ImGuiFileDialog::Instance()->OpenDialog(
            kOpenFile,
            "Select File",
            kFilters,
            config);
    }

    void AssetEditorPanel::FocusTab(IDocument* doc)
    {
        focused_tab_ = doc;
    }

    bool AssetEditorPanel::IsSelected(IDocument* doc) const
    {
        return selected_tab_ == doc;
    }

    void AssetEditorPanel::SaveSelectedTab() const
    {
        if (selected_tab_)
            selected_tab_->DumpAsset();
    }

    void AssetEditorPanel::SaveAllTabs() const
    {
        for (size_t i = 0; i < DocumentManager::Instance().DocumentCount(); ++i)
            DocumentManager::Instance().GetDocument(i).DumpAsset();
    }

    void AssetEditorPanel::SelectedTabUndo() const
    {
        if (selected_tab_)
            selected_tab_->Undo();
    }

    void AssetEditorPanel::SelectedTabRedo() const
    {
        if (selected_tab_)
            selected_tab_->Redo();
    }

    bool AssetEditorPanel::RequestShutdown()
    {
        CloseAllTabs(shutdown_unsaved_changes_modal_.pop);
        if (shutdown_unsaved_changes_modal_.pop.is_opening())
        {
            Open();
            GainFocus();
            return false;
        }
        else
            return true;
    }

    void AssetEditorPanel::PollShortcuts()
    {
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal))
            OpenFile();

        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
            SaveSelectedTab();

        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
            SaveAllTabs();

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
        {
            if (ImGui::Shortcut(
                ImGuiKey_Z | ImGuiMod_Ctrl,
                ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Repeat))
            {
                SelectedTabUndo();
            }

            if (ImGui::Shortcut(
                ImGuiKey_Z | ImGuiMod_Ctrl | ImGuiMod_Shift,
                ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_Repeat))
            {
                SelectedTabRedo();
            }
        }
    }

    void AssetEditorPanel::DrawTabBar()
    {
        ImGuiTabBarFlags tab_bar_flags =
            ImGuiTabBarFlags_AutoSelectNewTabs |
            ImGuiTabBarFlags_DrawSelectedOverline |
            ImGuiTabBarFlags_Reorderable;

        imtk::id_scope scope(this);
        if (auto _ = imtk::tab_bar("##AssetTabs", tab_bar_flags))
        {
            std::vector<size_t> closed;

            for (size_t i = 0; i < DocumentManager::Instance().DocumentCount(); IMP_NOP)
            {
                IDocument& doc = DocumentManager::Instance().GetDocument(i);
                if (doc.Exists())
                    ++i;
                else
                {
                    if (selected_tab_ == &doc)
                        selected_tab_ = nullptr;

                    if (focused_tab_ == &doc)
                        focused_tab_ = nullptr;

                    DocumentManager::Instance().Remove(i);
                }
            }

            IDocument* previously_selected_doc = selected_tab_;
            selected_tab_ = nullptr;
            std::unordered_set<IDocument*> seen_documents;

            for (size_t i = 0; i < DocumentManager::Instance().DocumentCount(); ++i)
            {
                scope.push(i);
                IDocument& doc = DocumentManager::Instance().GetDocument(i);
                seen_documents.insert(&doc);
                bool open = true;

                ImGuiTabItemFlags tab_item_flags = 0;
                if (doc.IsDirty())
                    tab_item_flags |= ImGuiTabItemFlags_UnsavedDocument;

                if (focused_tab_ == &doc)
                    tab_item_flags |= ImGuiTabItemFlags_SetSelected;

                if (auto _ = imtk::tab_item(doc.TabName() + "##" + std::to_string(i), tab_item_flags, &open))
                {
                    if (auto _ = imtk::context_menu::item("##AssetTabContextMenu"))
                    {
                        detail::ResourcePath path = doc.GetOlyPath();
                        if (detail::MetaSplitter::decode_meta(path).is_import())
                            path = path.get_source_path();

                        if (ImGui::MenuItem("Show in Content Browser"))
                            ContentBrowserPanel::ShowInContentBrowser(path);

                        if (ImGui::MenuItem("Reveal in explorer"))
                            PathInfo::RevealInExplorer(path.get_absolute(), false);
                    }

                    doc.Draw();
                    selected_tab_ = &doc;
                }

                if (!open)
                {
                    if (!doc.IsDirty())
                    {
                        closed.push_back(i);
                        if (&doc == selected_tab_)
                            selected_tab_ = nullptr;
                    }
                    else if (!pending_close_set_.contains(&doc))
                    {
                        pending_close_set_.insert(&doc);
                        pending_close_.push_back(&doc);
                        tab_unsaved_changes_modal_.pop.open();
                    }
                }
            }

            RemoveOldPendingDocuments(seen_documents);

            if (!window_unsaved_changes_modal_.pop.is_opening())
            {
                DrawTabUnsavedChangesModal(closed);
            }
            else
            {
                pending_close_.clear();
                pending_close_set_.clear();
            }

            std::sort(closed.begin(), closed.end());
            for (auto it = closed.rbegin(); it != closed.rend(); ++it)
                DocumentManager::Instance().Remove(*it);

            focused_tab_ = nullptr;
        }
    }

    void AssetEditorPanel::RemoveOldPendingDocuments(const std::unordered_set<IDocument*>& seen_documents)
    {
        for (auto it = pending_close_.begin(); it != pending_close_.end();)
        {
            if (seen_documents.contains(*it))
                ++it;
            else
                it = pending_close_.erase(it);
        }

        for (auto it = pending_close_set_.begin(); it != pending_close_set_.end();)
        {
            if (seen_documents.contains(*it))
                ++it;
            else
                it = pending_close_set_.erase(it);
        }
    }

    static imtk::unsaved_changes_modal::result DrawUnsavedChangesModalImpl(
        imtk::unsaved_changes_modal& modal,
        IDocument* doc)
    {
        if (!doc)
            return imtk::unsaved_changes_modal::result::no_draw;

        modal.description.clear();
        modal.description.push_back("Asset " + doc->TabName());
        modal.description.push_back("Full path: " + doc->GetOlyPath().string());
        const auto result = modal.draw();

        if (result == imtk::unsaved_changes_modal::result::save_changes)
            doc->DumpAsset();

        if (result == imtk::unsaved_changes_modal::result::discard_changes)
            doc->LoadAsset();

        return result;
    }

    void AssetEditorPanel::DrawTabUnsavedChangesModal(std::vector<size_t>& closed)
    {
        if (pending_close_.empty())
            return;

        IDocument* doc = pending_close_.front();
        const auto result = DrawUnsavedChangesModalImpl(tab_unsaved_changes_modal_, doc);

        if (tab_unsaved_changes_modal_.closing(result))
        {
            closed.push_back(DocumentManager::Instance().GetDocumentIndex(doc));
            if (doc == selected_tab_)
                selected_tab_ = nullptr;
        }

        if (result != imtk::unsaved_changes_modal::result::no_draw
            && result != imtk::unsaved_changes_modal::result::no_interaction)
        {
            pending_close_.erase(pending_close_.begin());
            pending_close_set_.erase(doc);
        }
    }

    bool AssetEditorPanel::DrawUnsavedChangesModal(imtk::unsaved_changes_modal& popup)
    {
        switch (DrawUnsavedChangesModalImpl(popup, selected_tab_))
        {
        case imtk::unsaved_changes_modal::result::save_changes:
        case imtk::unsaved_changes_modal::result::discard_changes:
            CloseAllTabs(popup.pop);
            return !popup.pop.is_opening();

        case imtk::unsaved_changes_modal::result::cancel_close:
            popup.pop.close();
            return false;

        default:
            return false;
        }
    }

    void AssetEditorPanel::CloseAllTabs(imtk::popup& popup)
    {
        pending_close_.clear();
        pending_close_set_.clear();
        popup.close();

        for (int i = DocumentManager::Instance().DocumentCount() - 1; i >= 0; --i)
        {
            IDocument& doc = DocumentManager::Instance().GetDocument(i);
            selected_tab_ = &doc;

            if (doc.IsDirty())
            {
                popup.open();
                break;
            }

            DocumentManager::Instance().Remove(i);
            selected_tab_ = nullptr;
        }
    }

    void AssetEditorPanel::DrawDefaultMenuBar()
    {
        if (auto _ = imtk::menu_bar())
        {
            if (auto _ = imtk::menu("File"))
            {
                if (ImGui::MenuItem("Open File", "Ctrl+O"))
                    OpenFile();
            }
        }
    }
}
