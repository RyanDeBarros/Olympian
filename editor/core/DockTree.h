#pragma once

#include <memory>
#include <typeindex>
#include <vector>

#include <imgui.h>

namespace oly::editor
{
	class PanelManager;

	class DockNode
	{
		std::vector<std::type_index> _indexes;
		std::unique_ptr<DockNode> _first = nullptr;
		std::unique_ptr<DockNode> _second = nullptr;
		ImGuiDir _direction = ImGuiDir_None;
		float _split_factor = 0.5f;

		DockNode() = default;

	public:
		static std::unique_ptr<DockNode> MakeBranch(ImGuiDir direction, std::unique_ptr<DockNode>&& first, std::unique_ptr<DockNode>&& second, float split_factor = 0.5f);
		static std::unique_ptr<DockNode> MakeLeaf(std::vector<std::type_index>&& indexes);

		void SplitLayout(ImGuiID id, PanelManager& panel_manager) const;
	};

	class DockTree
	{
		std::unique_ptr<DockNode> _root;

	public:
		DockTree(std::unique_ptr<DockNode>&& root);

		void SetupLayout(ImGuiID dockspace_id, PanelManager& panel_manager) const;
	};
}
