#pragma once

#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

namespace oly::editor
{
	class PanelManager;
	class DocumentManager;

	class MainMenuBar;

	struct Notification
	{
		std::string message;
		float timer = 3.f;
		float age = 0.f;
	};

	class MainWindow
	{
		bool _ui_initialized = false;
		ImGuiID _dockspace_id = 0;

		std::unique_ptr<PanelManager> _panel_manager;
		std::unique_ptr<DocumentManager> _document_manager;

		std::unique_ptr<MainMenuBar> _main_menu_bar;

		std::vector<Notification> _notifications;

	public:
		MainWindow();
		~MainWindow();

		static MainWindow& Instance();

	private:
		void Init();

	public:
		void Open();
		void Draw();

		PanelManager& GetPanelManager();
		DocumentManager& GetDocumentManager();

		MainMenuBar& GetMainMenuBar();

		void PushNotification(Notification&& notif);

	private:
		void DrawNotifications();
	};
}
