#include "MainWindow.h"

#include "core/editor/Editor.h"

#include "core/windows/MainMenuBar.h"

#include "documents/DocumentManager.h"
#include "documents/IDocument.h"

#include "panels/AssetEditorPanel.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/IPanel.h"
#include "panels/LogPanel.h"
#include "panels/PanelManager.h"
#include "panels/PreferencesPanel.h"
#include "panels/TreeViewPanel.h"

namespace oly::editor
{
    MainWindow::MainWindow()
        : panel_manager_(std::make_unique<PanelManager>())
        , document_manager_(std::make_unique<DocumentManager>())
        , main_menu_bar_(std::make_unique<MainMenuBar>())
    {
    }

    MainWindow::~MainWindow() = default;

    MainWindow& MainWindow::Instance()
    {
        return Editor::GetMainWindow();
    }

    void MainWindow::Open()
    {
        Editor::SetOSWindowMaximized(true);
    }

    void MainWindow::Draw()
    {
        if (!ui_initialized_)
        {
            Init();
            ui_initialized_ = true;
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

            main_menu_bar_->Draw();

            ImGui::DockSpace(dockspace_id_, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
            panel_manager_->Draw();
            DrawNotifications();
        }
    }

    PanelManager& MainWindow::GetPanelManager()
    {
        return *panel_manager_;
    }

    DocumentManager& MainWindow::GetDocumentManager()
    {
        return *document_manager_;
    }

    MainMenuBar& MainWindow::GetMainMenuBar()
    {
        return *main_menu_bar_;
    }

    void MainWindow::Init()
    {
        notif_handle_ = imtk::on_receive_notification().subscribe(
            [this](imtk::notification notif) {
                imtk::log(notif.level, notif.message);
                notifications_.push_back(std::move(notif));
            });

        panel_manager_->Add<AssetEditorPanel>().Open();
        panel_manager_->Add<ContentBrowserPanel>().Open();
        panel_manager_->Add<LogPanel>().Open();
        panel_manager_->Add<PreferencesPanel>().Close();
        panel_manager_->Add<TreeViewPanel>().Open();

        dockspace_id_ = ImGui::GetID("MainWindowDockspace");

        imtk::dock::make_branch(
            ImGuiDir_Down,
            imtk::dock::make_branch(
                ImGuiDir_Right,
                imtk::dock::make_leaf(
                    {
                        TreeViewPanel::Instance().GetTitle()
                    }),
                imtk::dock::make_leaf(
                    {
                        AssetEditorPanel::Instance().GetTitle(),
                        PreferencesPanel::Instance().GetTitle()
                    }),
                0.2f
            ),
            imtk::dock::make_leaf(
                {
                    LogPanel::Instance().GetTitle(),
                    ContentBrowserPanel::Instance().GetTitle()
                }),
            0.75f)->setup_layout(dockspace_id_);

        panel_manager_->Init();
        main_menu_bar_->Init();
    }

    void MainWindow::DrawNotifications()
    {
        for (size_t i = 0; i < notifications_.size(); ++i)
        {
            auto& notif = notifications_[i];

            imtk::style_var alpha_var(ImGuiStyleVar_Alpha, notif.age_alpha());
            const ImGuiWindowFlags flags =
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoInputs;

            if (auto _ = imtk::window("##notif" + std::to_string(i), flags))
            {
                if (auto _ = imtk::style_color(ImGuiCol_Text, imtk::log_level_color(notif.level)))
                    ImGui::TextUnformatted(notif.message.c_str());
            }

            notif.update();
        }
        
        auto it = std::remove_if(
            notifications_.begin(),
            notifications_.end(),
            [](const imtk::notification& notif) { return notif.expired(); });
        notifications_.erase(it, notifications_.end());
    }
}
