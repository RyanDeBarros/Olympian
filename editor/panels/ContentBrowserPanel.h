#pragma once

#include "panels/IPanel.h"

#include "assets/ResourcePath.h"

namespace oly::editor
{
	class ContentBrowserPanel : public IPanel
	{
		std::filesystem::path _folder;
		std::optional<std::filesystem::path> _selected_path;

	public:
		static ContentBrowserPanel& Instance();

		void InitImpl() override;
		const char* GetTitle() const override;
		void Draw() override;

		void ShowInContentBrowser(const detail::ResourcePath& path);
		void ShowInContentBrowser(const std::filesystem::path& path);

	private:
		void DrawFolderView();
		void DrawPathEntry(const std::filesystem::path& path, const char* label_override, const ImVec2 size);
	};
}
