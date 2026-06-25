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
		bool _window_unsaved_changes_modal = false;
		bool _shutdown_unsaved_changes_modal = false;
		bool _open_shutdown_modal = false;

	public:
		static AssetEditorPanel& Instance();

		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;

	private:
		void PollShortcuts();
		void DrawTabBar();
		void RemoveOldPendingDocuments(const std::unordered_set<IDocument*> seen_documents);
		void DrawTabUnsavedChangesModal(std::vector<size_t>& closed);

		bool DrawUnsavedChangesModal(bool& unsaved_changes_modal, const char* popup);
		void CloseAllTabs(bool& unsaved_changes_modal);

		void DrawDefaultMenuBar();

	public:
		void OpenFile();

		void FocusTab(IDocument* doc);
		bool IsSelected(IDocument* doc) const;

		void SaveSelectedTab() const;
		void SaveAllTabs() const;

		bool RequestShutdown();
	};
}
