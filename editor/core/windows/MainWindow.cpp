#include "MainWindow.h"

#include "core/editor/Editor.h"
#include "core/windows/MainMenuBar.h"
#include "core/editor/Logger.h"

#include "panels/PanelManager.h"
#include "panels/IPanel.h"
#include "panels/AssetEditorPanel.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/LogPanel.h"
#include "panels/PreferencesPanel.h"
#include "panels/TreeViewPanel.h"

#include "documents/DocumentManager.h"
#include "documents/IDocument.h"

namespace oly::editor
{
    MainWindow::MainWindow()
        : _panel_manager(std::make_unique<PanelManager>()),
        _document_manager(std::make_unique<DocumentManager>()),
        _main_menu_bar(std::make_unique<MainMenuBar>())
    {
    }

    MainWindow::~MainWindow() = default;

    MainWindow& MainWindow::Instance()
    {
        return Editor::GetMainWindow();
    }

    void MainWindow::Init()
    {
        _panel_manager->Add<AssetEditorPanel>().Open();
        _panel_manager->Add<ContentBrowserPanel>().Open();
        _panel_manager->Add<LogPanel>().Open();
        _panel_manager->Add<PreferencesPanel>().Close();
        _panel_manager->Add<TreeViewPanel>().Open();

        _dockspace_id = ImGui::GetID("MainWindowDockspace");

        imtk::dock::make_branch(
            ImGuiDir_Down,
            imtk::dock::make_branch(
                ImGuiDir_Right,
                imtk::dock::make_leaf({
                    TreeViewPanel::Instance().GetTitle()
                }),
                imtk::dock::make_leaf({
                    AssetEditorPanel::Instance().GetTitle(),
                    PreferencesPanel::Instance().GetTitle()
                }),
                0.2f
            ),
            imtk::dock::make_leaf({
                LogPanel::Instance().GetTitle(),
                ContentBrowserPanel::Instance().GetTitle()
            }),
            0.75f
        )->setup_layout(_dockspace_id);

        _panel_manager->Init();
        _main_menu_bar->Init();
    }

    void MainWindow::Open()
    {
        Editor::SetOSWindowMaximized(true);
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

        auto window_styling = imtk::style_stack().
            push(ImGuiStyleVar_WindowRounding, 0.0f).
            push(ImGuiStyleVar_WindowBorderSize, 0.0f).apply();

        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        if (auto _ = imtk::window("Main Window", window_flags))
        {
            window_styling.kill();

            _main_menu_bar->Draw();

            ImGui::DockSpace(_dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
            _panel_manager->Draw();
            DrawNotifications();
        }
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

    void MainWindow::PushNotification(Notification notif)
    {
        Logger::Log(notif.level, notif.message);
        _notifications.push_back(std::move(notif));
    }

    void MainWindow::DrawNotifications()
    {
        for (size_t i = 0; i < _notifications.size(); ++i)
        {
            Notification& notif = _notifications[i];

            float alpha = std::clamp(1.f - notif.age / notif.timer, 0.f, 1.f);
            imtk::style_var alpha_var(ImGuiStyleVar_Alpha, alpha);
            ImGuiWindowFlags flags =
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoInputs;

            if (auto _ = imtk::window("##notif" + std::to_string(i), flags))
            {
                if (auto _ = imtk::style_color(ImGuiCol_Text, LogLevelColor(notif.level)))
                    ImGui::TextUnformatted(notif.message.c_str());
            }

            notif.age += ImGui::GetIO().DeltaTime;
        }
        
        auto it = std::remove_if(_notifications.begin(), _notifications.end(), [](const Notification& notif) { return notif.age >= notif.timer; });
        _notifications.erase(it, _notifications.end());
    }
}
