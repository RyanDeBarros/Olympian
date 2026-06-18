#include "TreeViewPanel.h"

#include "core/editor/Editor.h"
#include "core/editor/Logger.h"
#include "core/editor/ProjectInfo.h"
#include "core/editor/ResourceLoader.h"
#include "core/editor/UID.h"

#include "core/windows/MainWindow.h"

#include "core/Errors.h"
#include "core/PathInfo.h"

#include "panels/PanelManager.h"
#include "panels/PreferencesPanel.h"

#include "gui/Toolbar.h"

#include <imgui.h>

#include <algorithm>
#include <stack>
#include <unordered_set>

namespace oly::editor
{
	TreeViewNode::TreeViewNode(std::filesystem::path path)
		: path(std::move(path))
	{
		Analyse();
	}

	void TreeViewNode::Analyse()
	{
		is_import = PathInfo::IsImportFile(this->path);
	}

	void TreeViewNode::Update()
	{
		const float update_interval = PreferencesPanel::Instance().GetSavedDesc().tree_view.advanced.analysis_interval.scratch;
		timer += ImGui::GetIO().DeltaTime;
		if (timer >= update_interval)
		{
			timer = fmod(timer, update_interval);
			Analyse();
		}
	}

	std::string TreeViewNode::Name() const
	{
		return path.filename().generic_string();
	}

	std::string TreeViewNode::DisplayName() const
	{
		const std::string name = Name();
		return path == ProjectInfo::Instance().ProjectRoot() ? name + " (" + path.generic_string() + ")" : name;
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
		{
			dropdown_open = false;
			if (IsFullyCollapsed())
				subnodes.clear();
		}
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

		std::ranges::sort(subnodes, [](const auto& a, const auto& b) {
			const bool adir = std::filesystem::is_directory(a->path);
			const bool bdir = std::filesystem::is_directory(b->path);

			if (adir != bdir)
				return adir > bdir;

			return a->path.filename() < b->path.filename();
		});
	}

	void TreeViewNode::CollapseAll()
	{
		dropdown_open = false;
		for (auto& subnode : subnodes)
			subnode->CollapseAll();
	}

	bool TreeViewNode::IsFullyCollapsed() const
	{
		if (dropdown_open)
			return false;

		for (auto& subnode : subnodes)
		{
			if (!subnode->IsFullyCollapsed())
				return false;
		}

		return true;
	}

	TreeViewPanel& TreeViewPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<TreeViewPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of TreeViewPanel");
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
		auto window = DrawDockedWindow(ImGuiWindowFlags_None);
		if (window.IsVisible())
		{
			DrawHeader();

			_root->Validate();

			std::stack<std::pair<TreeViewNode*, int>> process;
			process.push(std::make_pair(_root.get(), 0));
			int local_file_index = 0;

			while (!process.empty())
			{
				auto [node, indent] = process.top();
				process.pop();
				node->Update();

				if (!PassesFilter(*node))
					continue;

				DrawNode(*node, indent, local_file_index);

				if (node->IsBranching() && node->dropdown_open)
				{
					for (auto it = node->subnodes.rbegin(); it != node->subnodes.rend(); ++it)
						process.push(std::make_pair(it->get(), indent + 1));
				}
			}
		}
	}

	bool TreeViewPanel::PassesFilter(TreeViewNode& node) const
	{
		if (_config.ignore_imports && node.is_import)
			return false;

		if (node.Name()[0] == '.')
			return false;

		return true;
	}

	void TreeViewPanel::DrawHeader()
	{
		Toolbar::DrawIconToggleButton(IconResource::FilterOn, IconResource::FilterOff, _config.ignore_imports, "Ignore import files");
		
		ImGui::SameLine();
		if (Toolbar::DrawIconButton(IconResource::CollapseAll, "Collapse all", "##CollapseAll"))
			_root->CollapseAll();
		
		ImGui::Separator();
	}

	void TreeViewPanel::DrawNode(TreeViewNode& node, int indent, int& local_file_index)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent * ImGui::GetStyle().IndentSpacing);

		DrawRowBg(node, local_file_index);
		DrawNodePrefix(node);

		gui::IDScope scope(&node);
		ImGui::Selectable(node.DisplayName().c_str());

		if (ImGui::BeginDragDropSource())
		{
			std::string path = node.path.string();
			ImGui::SetDragDropPayload(StringID(UID::PathDrag), path.c_str(), path.size()); // TODO v9 make sure to include path drag drop source in content browser panel
			ImGui::Text("Drag path");
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginPopupContextItem("##NodeContextMenu"))
		{
			if (ImGui::MenuItem("Open"))
				node.Open();

			if (ImGui::MenuItem("Show in Content Browser"))
			{
				// TODO v9
			}

			if (ImGui::MenuItem("Reveal in Explorer"))
				PathInfo::RevealInExplorer(node.path);

			ImGui::EndPopup();
		}

		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				node.Open();
		}
	}

	void TreeViewPanel::DrawNodePrefix(TreeViewNode& node)
	{
		if (node.IsBranching())
		{
			gui::IDScope scope(&node);
			if (node.dropdown_open)
			{
				if (ImGui::ArrowButton("##Dropdown", ImGuiDir_Down))
					node.CloseBranch();
			}
			else
			{
				if (ImGui::ArrowButton("##Dropdown", ImGuiDir_Right))
					node.OpenBranch();
			}
			ImGui::SameLine();
		}
		else
		{
			ImGui::Dummy(ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
			ImGui::SameLine();
		}
	}

	void TreeViewPanel::DrawRowBg(TreeViewNode& node, int& local_file_index)
	{
		ImVec2 start = ImGui::GetCursorScreenPos();
		ImVec2 end = start + ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());

		if (node.IsBranching())
		{
			ImGui::GetWindowDrawList()->AddRectFilled(start, end, ImGui::GetColorU32(ImGuiCol_Header, 0.6f), 6.0f);
			local_file_index = 0;
		}
		else
		{
			if (local_file_index % 2 == 1)
				ImGui::GetWindowDrawList()->AddRectFilled(start, end, ImGui::GetColorU32(ImGuiCol_FrameBg, 0.15f));

			++local_file_index;
		}
	}
}
