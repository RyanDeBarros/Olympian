#pragma once

#include "panels/IPanel.h"

#include <filesystem>

namespace oly::editor
{
	struct TreeViewNode
	{
		std::filesystem::path path;
		std::vector<std::unique_ptr<TreeViewNode>> subnodes;
		bool dropdown_open = false;

		std::string DisplayName() const;
		void Validate();
		bool IsBranching() const;
		void Open();
		void OpenBranch();
		void CloseBranch();
		void RefreshSubnodes();
	};

	class TreeViewPanel : public IPanel
	{
		std::unique_ptr<TreeViewNode> _root;

	public:
		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;
	};
}
