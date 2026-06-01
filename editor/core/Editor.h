#pragma once

#include "GL.h"

#include <filesystem>
#include <memory>

namespace oly::editor
{
	class ProjectSelectWindow;
	class Logger;
	class MainWindow;
	class ShortcutManager;
	class ProjectInfo;

	enum class AppState
	{
		ProjectSelect,
		Main
	};

	class Editor
	{
		GLFWwindow* _os_window = nullptr;
		AppState _app_state = AppState::ProjectSelect;

		std::unique_ptr<ProjectSelectWindow> _project_select_window;

		std::unique_ptr<Logger> _logger;
		std::unique_ptr<MainWindow> _main_window;
		std::unique_ptr<ShortcutManager> _shortcut_manager;
		std::unique_ptr<ProjectInfo> _project_info;

		Editor();
		Editor(const Editor&) = delete;
		Editor(Editor&&) = delete;

	public:
		static Editor& Instance();
		void Init(GLFWwindow* window);
		void Tick();

		void SetOSWindowSize(int width, int height);

		AppState GetAppState() const;
		ProjectSelectWindow& GetProjectSelectWindow();
		Logger& GetLogger();
		MainWindow& GetMainWindow();
		ShortcutManager& GetShortcutManager();
		ProjectInfo& GetProjectInfo();

		void OpenProject(const std::filesystem::path& path);
		void OpenFile(const std::filesystem::path& path);
	};
}
