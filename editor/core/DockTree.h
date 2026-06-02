#pragma once

#include <typeindex>
#include <vector>

#include <imgui.h>

namespace oly::editor
{
	class PanelManager;

	struct DockNode
	{
		std::vector<std::type_index> indexes;
		DockNode* first = nullptr;
		DockNode* second = nullptr;
		bool horizontal = true;
		float split_factor = 0.5f;

		void SplitLayout(ImGuiID id, PanelManager& panel_manager) const;
	};

	struct DockTree
	{
		DockNode root;

		void SetupLayout(ImGuiID dockspace_id, PanelManager& panel_manager) const;
	};
}
