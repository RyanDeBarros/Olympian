#include "DockTree.h"

#include <imgui_internal.h>

#include "panels/PanelManager.h"
#include "panels/IPanel.h"

namespace oly::editor
{
    void DockNode::SplitLayout(ImGuiID id, PanelManager& panel_manager) const
    {
        if (!_indexes.empty())
        {
            for (std::type_index index : _indexes)
                ImGui::DockBuilderDockWindow(panel_manager.Get(index)->GetTitle(), id);
        }
        else if (_first && _second)
        {
            ImGuiID dock_first, dock_second;
            dock_second = ImGui::DockBuilderSplitNode(id, _direction, 1.f - _split_factor, nullptr, &dock_first);
            _first->SplitLayout(dock_first, panel_manager);
            _second->SplitLayout(dock_second, panel_manager);
        }
    }

    std::unique_ptr<DockNode> DockNode::MakeBranch(ImGuiDir direction, std::unique_ptr<DockNode>&& first, std::unique_ptr<DockNode>&& second, float split_factor)
    {
        DockNode node;
        node._direction = direction;
        node._first = std::move(first);
        node._second = std::move(second);
        node._split_factor = split_factor;
        return std::make_unique<DockNode>(std::move(node));
    }

    std::unique_ptr<DockNode> DockNode::MakeLeaf(std::vector<std::type_index>&& indexes)
    {
        DockNode node;
        node._indexes = std::move(indexes);
        return std::make_unique<DockNode>(std::move(node));
    }

    DockTree::DockTree(std::unique_ptr<DockNode>&& root)
        : _root(std::move(root))
    {
    }

    void DockTree::SetupLayout(ImGuiID dockspace_id, PanelManager& panel_manager) const
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);
        _root->SplitLayout(dockspace_id, panel_manager);
        ImGui::DockBuilderFinish(dockspace_id);
    }
}
