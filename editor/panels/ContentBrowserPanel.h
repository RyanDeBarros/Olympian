#pragma once

#include "panels/IPanel.h"

#include "core/SpecialUndoActions.h"

#include "assets/ResourcePath.h"
#include "assets/KeyDecl.h"

#include <imtk.hpp>

#include <imp/event.hpp>
#include <imp/timeline_queue.hpp>
#include <imp/undo_history.hpp>

#include <set>

namespace oly::editor
{
	class ContentBrowserPanel : public IPanel
	{
		std::filesystem::path _folder;
		imtk::w::icon_button _favorited;
		bool _on_res_root = true;
		std::vector<std::filesystem::path> _selectable_entry_paths;
		std::vector<std::filesystem::path> _selected_paths;
		std::optional<std::filesystem::path> _active_selected_path;
		imp::timeline_queue<std::filesystem::path> _folder_history;
		imp::undo_history _undo_history;
		std::string _rename_buffer;
		imp::event_listener _listener;

		struct NewAssetInfo
		{
			imtk::popup popup;
			detail::Key type;
			std::string name = "";

			NewAssetInfo(detail::Key type, std::string name, const char* popup_label);
		};

		std::optional<NewAssetInfo> _new_asset;

		struct ImportFolderInfo
		{
			imtk::popup popup;
			std::filesystem::path folder;

			ImportFolderInfo(std::filesystem::path folder);
		};

		std::optional<ImportFolderInfo> _import_folder;

		struct PruneFolderInfo
		{
			imtk::popup popup;
			std::filesystem::path folder;

			PruneFolderInfo(std::filesystem::path folder);
		};

		std::optional<PruneFolderInfo> _prune_folder;

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
		void DrawMainToolbar(CompoundUndoActionQueue& fio_queue);

		void SetFolder(std::filesystem::path folder);
		void SwitchFolder(std::filesystem::path folder);

		std::set<detail::ResourcePath>& GetFavoritesList() const;
		bool ShouldBeFavorited() const;
		void SyncFavoritesList() const;
		void DrawFavoritesList();

		void DrawFolderView(CompoundUndoActionQueue& fio_queue);
		void DrawPathTable(CompoundUndoActionQueue& fio_queue);

		struct EntryTableState;

		void DrawPathEntry(const std::filesystem::path& path, bool dotdot, const EntryTableState& entry_table_state, CompoundUndoActionQueue& fio_queue);
		ImVec2 FitPathLabel(std::string& label, const float width);
		void OpenPath(const std::filesystem::path& path);

		void NewAssetMenu();
		void NewFolderMenu();
		void DrawNewAssetPopups(CompoundUndoActionQueue& fio_queue);
		void CreateNewAsset(CompoundUndoActionQueue& fio_queue);

		void ClearSelection();
		void PruneSelection();
		void ClickSelect(const std::filesystem::path& path);
		bool IsSelected(const std::filesystem::path& path) const;
		bool IsOnlySelected(const std::filesystem::path& path) const;

		void DeletePath(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue) const;
		
		void ImportFromPath(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue);
		void ImportFile(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue);
		void DrawImportFolderPopup(CompoundUndoActionQueue& fio_queue);

		void PruneFromPath(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue);
		void PrunePath(const std::filesystem::path& path, CompoundUndoActionQueue& fio_queue);
		void DrawPruneFolderPopup(CompoundUndoActionQueue& fio_queue);
	};

	struct ContentBrowserPathDDP : public imtk::drag_droppable
	{
		std::string path;

		ContentBrowserPathDDP(std::string path);

		void send(const std::function<void(const void*, size_t)>& dump) const override;
	};
}

template<>
struct imtk::drag_drop_convert<oly::editor::ContentBrowserPathDDP>
{
	using payload_view = std::string_view;
	payload_view view(const void* buf, size_t size) const;
};
