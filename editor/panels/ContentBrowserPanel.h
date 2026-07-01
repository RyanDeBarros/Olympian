#pragma once

#include "panels/IPanel.h"

#include "assets/ResourcePath.h"

namespace oly::editor
{
	class ContentBrowserPanel : public IPanel
	{
		std::filesystem::path _folder;

	public:
		static ContentBrowserPanel& Instance();

		void InitImpl() override;
		const char* GetTitle() const override;
		void Draw() override;

		void ShowInContentBrowser(const detail::ResourcePath& path);

	private:
		void DrawFolderView();
	};
}
