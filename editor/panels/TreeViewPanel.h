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
		bool is_import = false;
		float timer = 0.f;

		TreeViewNode(std::filesystem::path path);

		void Analyse();
		void Update();
		std::string DisplayName() const;
		void Validate();
		bool IsBranching() const;
		void Open();
		void OpenBranch();
		void CloseBranch();
		void RefreshSubnodes();
		void CollapseAll();
	};

	struct TreeViewConfig
	{
		bool ignore_imports = true;
	};

	class TreeViewPanel : public IPanel
	{
		std::unique_ptr<TreeViewNode> _root;
		TreeViewConfig _config;

	public:
		void Init() override;
		const char* GetTitle() const override;
		void Draw() override;

	private:
		bool PassesFilter(TreeViewNode& node) const;

		void DrawHeader();
		void DrawNode(TreeViewNode& node, int indent, int& local_file_index);
		void DrawNodePrefix(TreeViewNode& node);
		void DrawRowBg(TreeViewNode& node, int& local_file_index);
	};
}
