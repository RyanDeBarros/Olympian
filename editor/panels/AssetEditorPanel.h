#pragma once

#include "panels/IPanel.h"

#include <unordered_set>

namespace oly::editor
{
    class IDocument;

    class AssetEditorPanel : public IPanel
    {
        IDocument* selected_tab_ = nullptr;
        IDocument* focused_tab_ = nullptr;
        std::vector<IDocument*> pending_close_;
        std::unordered_set<IDocument*> pending_close_set_;
        imtk::unsaved_changes_modal tab_unsaved_changes_modal_;
        imtk::unsaved_changes_modal window_unsaved_changes_modal_;
        imtk::unsaved_changes_modal shutdown_unsaved_changes_modal_;

    public:
        AssetEditorPanel();

    public:
        void InitImpl() override;
        const char* GetTitle() const override;
        void Draw() override;

    public:
        static AssetEditorPanel& Instance();

        void OpenFile();

        void FocusTab(IDocument* doc);
        bool IsSelected(IDocument* doc) const;

        void SaveSelectedTab() const;
        void SaveAllTabs() const;

        void SelectedTabUndo() const;
        void SelectedTabRedo() const;

        bool RequestShutdown();

    private:
        void PollShortcuts();
        void DrawTabBar();
        void RemoveOldPendingDocuments(const std::unordered_set<IDocument*>& seen_documents);
        void DrawTabUnsavedChangesModal(std::vector<size_t>& closed);

        bool DrawUnsavedChangesModal(imtk::unsaved_changes_modal& popup);
        void CloseAllTabs(imtk::popup& popup);

        void DrawDefaultMenuBar();
    };
}
