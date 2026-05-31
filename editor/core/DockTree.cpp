#include "DockTree.h"

#include <imgui_internal.h>

#include "panels/PanelManager.h"
#include "panels/IPanel.h"

namespace oly::editor
{
    void DockNode::SplitLayout(ImGuiID id, PanelManager& panel_manager) const
    {
        if (index)
            ImGui::DockBuilderDockWindow(panel_manager.Get(*index)->GetTitle(), id);
        else if (!first || !second)
            return;
        else if (horizontal)
        {
            ImGuiID dock_left, dock_right;
            dock_left = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, split_factor, nullptr, &dock_right);
            first->SplitLayout(dock_left, panel_manager);
            second->SplitLayout(dock_right, panel_manager);
        }
        else
        {
            ImGuiID dock_top, dock_bottom;
            dock_top = ImGui::DockBuilderSplitNode(id, ImGuiDir_Up, split_factor, nullptr, &dock_bottom);
            first->SplitLayout(dock_top, panel_manager);
            second->SplitLayout(dock_bottom, panel_manager);
        }
    }

    void DockTree::SetupLayout(ImGuiID dockspace_id, PanelManager& panel_manager) const
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);
        root.SplitLayout(dockspace_id, panel_manager);
        ImGui::DockBuilderFinish(dockspace_id);
    }
}
