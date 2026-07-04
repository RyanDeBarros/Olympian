#pragma once

#include "panels/IPanel.h"

#include "core/UndoHistory.h"

#include "assets/ResourcePath.h"
#include "util/TimelineQueue.h"

#include <set>

namespace oly::editor
{
	class ContentBrowserPanel : public IPanel
	{
		std::filesystem::path _folder;
		bool _favorited = false;
		bool _on_res_root = true;
		std::optional<std::filesystem::path> _selected_path;
		TimelineQueue<std::filesystem::path> _folder_history;
		UndoHistory _undo_history;
		std::string _rename_buffer;

		struct
		{
			bool new_signal = false;
			std::string new_signal_name = "";
		} _asset_popups;

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
		void DrawPathEntry(const std::filesystem::path& path, bool dotdot, const ImVec2 size, std::vector<std::unique_ptr<UndoAction>>& fio_operations);
		ImVec2 FitPathLabel(std::string& label, const float width);
		void OpenPath(const std::filesystem::path& path);

		void NewAssetMenu();
		void DrawNewAssetPopups();
	};
}
