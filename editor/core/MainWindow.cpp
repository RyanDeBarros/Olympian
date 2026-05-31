#include "MainWindow.h"

#include "Editor.h"
#include "DockTree.h"

#include "documents/DocumentManager.h"
#include "documents/IDocument.h"

// TODO v7 remove
#include "documents/TextureDocument.h"
#include "documents/SpriteDocument.h"

#include "panels/PanelManager.h"
#include "panels/IPanel.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/AssetEditorPanel.h"

#include "MainMenuBar.h"

MainWindow::MainWindow()
    : _panel_manager(std::make_unique<PanelManager>()),
    _document_manager(std::make_unique<DocumentManager>()),
    _main_menu_bar(std::make_unique<MainMenuBar>())
{
}

MainWindow::~MainWindow() = default;

MainWindow& MainWindow::Instance()
{
    return Editor::Instance().GetMainWindow();
}

void MainWindow::Init()
{
    _panel_manager->Add<ContentBrowserPanel>().Open();
    _panel_manager->Add<AssetEditorPanel>().Open();

    _dockspace_id = ImGui::GetID("MainWindowDockspace");

    DockTree tree;

    DockNode content_browser;
    content_browser.index = typeid(ContentBrowserPanel);

    DockNode asset_editor;
    asset_editor.index = typeid(AssetEditorPanel);

    tree.root.horizontal = false;
    tree.root.first = &asset_editor;
    tree.root.second = &content_browser;

    tree.SetupLayout(_dockspace_id, *_panel_manager);


    // TODO v7 remove
    _document_manager->Add<TextureDocument>();
    _document_manager->Add<SpriteDocument>();
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

    _main_menu_bar->Draw();

    ImGui::DockSpace(_dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    _panel_manager->Draw();
	ImGui::End();
}

PanelManager& MainWindow::GetPanelManager()
{
    return *_panel_manager;
}

DocumentManager& MainWindow::GetDocumentManager()
{
    return *_document_manager;
}
