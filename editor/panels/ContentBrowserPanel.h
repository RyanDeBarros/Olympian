#pragma once

#include "panels/IPanel.h"

#include "core/UndoHistory.h"

#include "assets/ResourcePath.h"
#include "assets/KeyDecl.h"
#include "util/TimelineQueue.h"
#include "util/FunctionalEvent.h"

#include <set>

namespace oly::editor
{
	class ContentBrowserPanel : public IPanel
	{
		std::filesystem::path _folder;
		bool _favorited = false;
		bool _on_res_root = true;
		std::vector<std::filesystem::path> _selectable_entry_paths;
		std::vector<std::filesystem::path> _selected_paths;
		std::optional<std::filesystem::path> _active_selected_path;
		TimelineQueue<std::filesystem::path> _folder_history;
		UndoHistory _undo_history;
		std::string _rename_buffer;
		FunctionalEvent<>::Handle _listener;

		struct NewAssetInfo
		{
			bool pending_popup = true;
			detail::Key type;
			std::string name = "";
			const char* popup;

			NewAssetInfo(detail::Key type, std::string name, const char* popup);
		};

		std::optional<NewAssetInfo> _new_asset;

	public:
		ContentBrowserPanel();
		static ContentBrowserPanel& Instance();

		void InitImpl() override;
		const char* GetTitle() const override;
		void Draw() override;

		static ContentBrowserPanel& FocusInstance();
		static void ShowInContentBrowser(const detail::ResourcePath& path);
		static void ShowInContentBrowser(const std::filesystem::path& path);

	private:
		void DrawMainToolbar();

		void SetFolder(std::filesystem::path folder);
		void SwitchFolder(std::filesystem::path folder);

		std::set<detail::ResourcePath>& GetFavoritesList() const;
		bool ShouldBeFavorited() const;
		void SyncFavoritesList() const;
		void DrawFavoritesList();

		void DrawFolderView(std::vector<std::unique_ptr<UndoAction>>& fio_operations);
		void DrawPathTable(std::vector<std::unique_ptr<UndoAction>>& fio_operations);

		struct EntryTableState;

		void DrawPathEntry(const std::filesystem::path& path, bool dotdot, const EntryTableState& entry_table_state, std::vector<std::unique_ptr<UndoAction>>& fio_operations);
		ImVec2 FitPathLabel(std::string& label, const float width);
		void OpenPath(const std::filesystem::path& path);

		void NewAssetMenu();
		void DrawNewAssetPopups(std::vector<std::unique_ptr<UndoAction>>& fio_operations);

		void ClearSelection();
		void PruneSelection();
		void ClickSelect(const std::filesystem::path& path);
		bool IsSelected(const std::filesystem::path& path) const;
		bool IsOnlySelected(const std::filesystem::path& path) const;

		void DeletePath(const std::filesystem::path& path, std::vector<std::unique_ptr<UndoAction>>& fio_operations) const;
		void ExecuteFIO(std::vector<std::unique_ptr<UndoAction>>& fio_operations);

		bool InitNewAsset(const std::filesystem::path& path, detail::Key meta_type);
	};
}
