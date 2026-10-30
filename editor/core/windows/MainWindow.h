#pragma once

#include <imtk.hpp>

namespace oly::editor
{
    class PanelManager;
    class DocumentManager;

    class MainMenuBar;

    enum class LogLevel
        : int;

    class MainWindow
    {
        bool ui_initialized_ = false;
        ImGuiID dockspace_id_ = 0;

        std::unique_ptr<PanelManager> panel_manager_;
        std::unique_ptr<DocumentManager> document_manager_;

        std::unique_ptr<MainMenuBar> main_menu_bar_;

        imp::event_listener notif_handle_;
        std::vector<imtk::notification> notifications_;

    public:
        MainWindow();
        ~MainWindow();

    public:
        static MainWindow& Instance();

        void Open();
        void Draw();

        PanelManager& GetPanelManager();
        DocumentManager& GetDocumentManager();

        MainMenuBar& GetMainMenuBar();

    private:
        void Init();
        void DrawNotifications();
    };
}
