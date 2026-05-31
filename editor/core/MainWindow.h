#pragma once

#include <imgui.h>

#include <memory>

namespace oly::editor
{
	class PanelManager;
	class DocumentManager;

	class MainMenuBar;

	class MainWindow
	{
		bool _ui_initialized = false;
		ImGuiID _dockspace_id = 0;

		std::unique_ptr<PanelManager> _panel_manager;
		std::unique_ptr<DocumentManager> _document_manager;

		std::unique_ptr<MainMenuBar> _main_menu_bar;

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
	};
}
