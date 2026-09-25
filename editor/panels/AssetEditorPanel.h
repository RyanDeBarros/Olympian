#pragma once

#include "panels/IPanel.h"

#include <unordered_set>

namespace oly::editor
{
	class IDocument;

	class AssetEditorPanel : public IPanel
	{
		IDocument* _selected_tab = nullptr;
		IDocument* _focused_tab = nullptr;
		std::vector<IDocument*> _pending_close;
		std::unordered_set<IDocument*> _pending_close_set;
		imtk::unsaved_changes_modal _tab_unsaved_changes_modal;
		imtk::unsaved_changes_modal _window_unsaved_changes_modal;
		imtk::unsaved_changes_modal _shutdown_unsaved_changes_modal;

	public:
		static AssetEditorPanel& Instance();

		AssetEditorPanel();

		void InitImpl() override;
		const char* GetTitle() const override;
		void Draw() override;

	private:
		void PollShortcuts();
		void DrawTabBar();
		void RemoveOldPendingDocuments(const std::unordered_set<IDocument*>& seen_documents);
		void DrawTabUnsavedChangesModal(std::vector<size_t>& closed);

		bool DrawUnsavedChangesModal(imtk::unsaved_changes_modal& popup);
		void CloseAllTabs(imtk::popup& popup);

		void DrawDefaultMenuBar();

	public:
		void OpenFile();

		void FocusTab(IDocument* doc);
		bool IsSelected(IDocument* doc) const;

		void SaveSelectedTab() const;
		void SaveAllTabs() const;

		void SelectedTabUndo() const;
		void SelectedTabRedo() const;

		bool RequestShutdown();
	};
}
