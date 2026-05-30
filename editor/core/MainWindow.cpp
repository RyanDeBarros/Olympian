#include "MainWindow.h"

#include "panels/PanelManager.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/AssetEditorPanel.h"

#include <imgui_internal.h>

MainWindow::MainWindow()
    : _panel_manager(std::make_unique<PanelManager>())
{
}

MainWindow::~MainWindow() = default;

void MainWindow::Init()
{
    _panel_manager->Add<ContentBrowserPanel>().Open();
    _panel_manager->Add<AssetEditorPanel>().Open();

    _dockspace_id = ImGui::GetID("MainWindowDockspace");

    ImGui::DockBuilderRemoveNode(_dockspace_id);
    ImGui::DockBuilderAddNode(_dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(_dockspace_id, ImGui::GetMainViewport()->WorkSize);

    ImGuiID dock_main = _dockspace_id;

    ImGuiID dock_top, dock_bottom;
    dock_top = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Up, 0.5f, nullptr, &dock_bottom);

    ImGui::DockBuilderDockWindow(_panel_manager->Get<ContentBrowserPanel>()->GetTitle(), dock_bottom);
    ImGui::DockBuilderDockWindow(_panel_manager->Get<AssetEditorPanel>()->GetTitle(), dock_top);

    ImGui::DockBuilderFinish(_dockspace_id);
}

void MainWindow::Draw()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

	ImGui::Begin("Main Window", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGui::DockSpace(_dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    _panel_manager->Draw();
	ImGui::End();
}
