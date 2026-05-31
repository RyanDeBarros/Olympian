#pragma once

#include <optional>
#include <typeindex>

#include <imgui.h>

class PanelManager;

struct DockNode
{
	std::optional<std::type_index> index;
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
