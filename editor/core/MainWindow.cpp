#include "MainWindow.h"

#include "core/Editor.h"
#include "core/DockTree.h"
#include "core/MainMenuBar.h"
#include "core/Logger.h"

#include "panels/PanelManager.h"
#include "panels/IPanel.h"
#include "panels/AssetEditorPanel.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/LogPanel.h"
#include "panels/TreeViewPanel.h"

#include "documents/DocumentManager.h"
#include "documents/IDocument.h"

namespace oly::editor
{
    Notification::Notification(LogLevel level, std::string&& message, float timer)
        : level(level), message(std::move(message)), timer(timer)
    {
    }

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
        _panel_manager->Add<LogPanel>().Open();
        _panel_manager->Add<TreeViewPanel>().Open();

        _dockspace_id = ImGui::GetID("MainWindowDockspace");

        DockTree tree = DockNode::MakeBranch(
            ImGuiDir_Down,
            DockNode::MakeBranch(
                ImGuiDir_Right,
                DockNode::MakeLeaf({
                    typeid(TreeViewPanel)
                }),
                DockNode::MakeLeaf({
                    typeid(AssetEditorPanel)
                }),
                0.2f
            ),
            DockNode::MakeLeaf({
                typeid(ContentBrowserPanel),
                typeid(LogPanel)
            }),
            0.75f
        );

        tree.SetupLayout(_dockspace_id, *_panel_manager);

        _main_menu_bar->Init();
    }

    void MainWindow::Open()
    {
        Editor::Instance().SetOSWindowMaximized(true);
    }

    void MainWindow::Draw()
    {
        if (!_ui_initialized)
        {
            Init();
            _ui_initialized = true;
        }

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
        DrawNotifications();
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

    MainMenuBar& MainWindow::GetMainMenuBar()
    {
        return *_main_menu_bar;
    }

    void MainWindow::PushNotification(Notification&& notif)
    {
        Logger::Instance().Log(notif.level, notif.message.c_str());
        _notifications.push_back(std::move(notif));
    }

    void MainWindow::DrawNotifications()
    {
        for (size_t i = 0; i < _notifications.size(); ++i)
        {
            Notification& notif = _notifications[i];

            float alpha = std::clamp(1.f - notif.age / notif.timer, 0.f, 1.f);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            ImGuiWindowFlags flags =
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoInputs;

            ImGui::Begin(("##notif" + std::to_string(i)).c_str(), nullptr, flags);
            ImGui::PushStyleColor(ImGuiCol_Text, LogLevelColor(notif.level));
            ImGui::TextUnformatted(notif.message.c_str());
            ImGui::PopStyleColor();
            ImGui::End();
            ImGui::PopStyleVar();

            notif.age += ImGui::GetIO().DeltaTime;
        }
        
        auto it = std::remove_if(_notifications.begin(), _notifications.end(), [](const Notification& notif) { return notif.age >= notif.timer; });
        _notifications.erase(it, _notifications.end());
    }
}
