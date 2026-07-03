#pragma once

#include "panels/IPanel.h"

#include "assets/ResourcePath.h"

#include <set>

namespace oly::editor
{
	class ContentBrowserPanel : public IPanel
	{
		std::filesystem::path _folder;
		bool _favorited = false;
		bool _on_res_root = true;
		std::optional<std::filesystem::path> _selected_path;

	public:
		static ContentBrowserPanel& Instance();

		void InitImpl() override;
		const char* GetTitle() const override;
		void Draw() override;

		static ContentBrowserPanel& FocusInstance();
		static void ShowInContentBrowser(const detail::ResourcePath& path);
		static void ShowInContentBrowser(const std::filesystem::path& path);

	private:
		void SetFolder(std::filesystem::path folder);

		std::set<detail::ResourcePath>& GetFavoritesList() const;
		bool ShouldBeFavorited() const;
		void SyncFavoritesList() const;
		void DrawFavoritesList();

		void DrawFolderView();
		void DrawPathTable();
		void DrawPathEntry(const std::filesystem::path& path, bool dotdot, const ImVec2 size);
		ImVec2 FitPathLabel(std::string& label, const float width);
		void OpenPath(const std::filesystem::path& path);
	};
}
