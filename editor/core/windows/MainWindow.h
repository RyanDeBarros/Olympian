#pragma once

#include <imtk.hpp>

namespace oly::editor
{
	class PanelManager;
	class DocumentManager;

	class MainMenuBar;

	enum class LogLevel : int;

	class MainWindow
	{
		bool _ui_initialized = false;
		ImGuiID _dockspace_id = 0;

		std::unique_ptr<PanelManager> _panel_manager;
		std::unique_ptr<DocumentManager> _document_manager;

		std::unique_ptr<MainMenuBar> _main_menu_bar;

		// TODO v9.3 rename imp::functional_event to imp::event and imp::functional_event<...>::handle to just imp::event_listener
		imp::functional_event<imtk::notification>::handle _notif_handle;
		std::vector<imtk::notification> _notifications;

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

	private:
		void DrawNotifications();
	};
}
