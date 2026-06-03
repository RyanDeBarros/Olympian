#include "TreeViewPanel.h"

#include "core/Editor.h"
#include "core/Logger.h"
#include "core/ProjectInfo.h"

#include <imgui.h>

#include <algorithm>
#include <unordered_set>

namespace oly::editor
{
	std::string TreeViewNode::DisplayName() const
	{
		return path.filename().generic_string();
	}

	void TreeViewNode::Validate()
	{
		if (!std::filesystem::exists(path))
		{
			subnodes.clear();
			return;
		}

		if (IsBranching() && dropdown_open)
			RefreshSubnodes();

		for (auto& subnode : subnodes)
			subnode->Validate();
	}

	bool TreeViewNode::IsBranching() const
	{
		return std::filesystem::is_directory(path);
	}

	void TreeViewNode::Open()
	{
		if (std::filesystem::is_regular_file(path))
			Editor::Instance().OpenFile(path);
		else if (std::filesystem::is_directory(path))
			OpenBranch();
	}

	void TreeViewNode::OpenBranch()
	{
		if (IsBranching() && !dropdown_open)
		{
			dropdown_open = true;
			RefreshSubnodes();
		}
	}

	void TreeViewNode::CloseBranch()
	{
		if (IsBranching() && dropdown_open)
			dropdown_open = false;
	}

	void TreeViewNode::RefreshSubnodes()
	{
		std::unordered_set<std::filesystem::path> existing;
		std::error_code ec;
		for (const auto& entry : std::filesystem::directory_iterator(path, std::filesystem::directory_options::skip_permission_denied, ec))
		{
			if (ec)
			{
				Logger::Instance().Log(LogLevel::Error, ("TreeViewNode::RefreshSubnodes(): " + ec.message()).c_str());
				subnodes.clear();
				return;
			}

			existing.insert(entry.path());
		}

		// remove old paths
		for (auto it = subnodes.begin(); it != subnodes.end();)
		{
			auto eit = existing.find((*it)->path);
			if (eit != existing.end())
			{
				existing.erase(eit);
				++it;
			}
			else
				it = subnodes.erase(it);
		}

		// add new paths
		for (auto it = existing.begin(); it != existing.end(); ++it)
			subnodes.push_back(std::make_unique<TreeViewNode>(*it));

		std::ranges::sort(subnodes, {}, [](const auto& p) { return p->path; });
	}

	void TreeViewPanel::Init()
	{
		_root = std::make_unique<TreeViewNode>(ProjectInfo::Instance().ProjectRoot());
	}

	const char* TreeViewPanel::GetTitle() const
	{
		return "Tree View";
	}

	void TreeViewPanel::Draw()
	{
		ImGui::Begin(GetTitle());
		// TODO v8
		ImGui::End();
	}
}
