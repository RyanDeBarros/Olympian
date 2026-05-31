#pragma once

#include <filesystem>
#include <memory>

namespace oly::editor
{
	class Logger;
	class MainWindow;
	class ShortcutManager;
	class ProjectInfo;

	class Editor
	{
		bool _ui_initialized = false;
		std::unique_ptr<Logger> _logger;
		std::unique_ptr<MainWindow> _main_window;
		std::unique_ptr<ShortcutManager> _shortcut_manager;
		std::unique_ptr<ProjectInfo> _project_info;

		Editor();
		Editor(const Editor&) = delete;
		Editor(Editor&&) = delete;

	public:
		static Editor& Instance();
		void Init();
		void Tick();

		Logger& GetLogger();
		MainWindow& GetMainWindow();
		ShortcutManager& GetShortcutManager();
		ProjectInfo& GetProjectInfo();

		void OpenFile(const std::filesystem::path& path);
	};
}
