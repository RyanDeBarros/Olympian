#pragma once

#include "external/GL.h"

#include "util/FunctionalEvent.h"

#include <imtk.hpp>
#include <imtk/os_window.hpp>

#include <filesystem>
#include <memory>

namespace oly::editor
{
	class ProjectSelectWindow;
	class Logger;
	class MainWindow;
	class ShortcutManager;
	class ProjectInfo;
	class PreferencesDesc;
	class LiveSettings;
	class LiveSettingsDesc;

	enum class AppState
	{
		ProjectSelect,
		Main
	};

	class Editor
	{
		// TODO v9.3 own os_window. Make Editor an active instance singleton instead of this static singleton
		imtk::os_window* _os_window = nullptr;
		AppState _app_state = AppState::ProjectSelect;

		std::unique_ptr<ProjectSelectWindow> _project_select_window;

		std::unique_ptr<Logger> _logger;
		std::unique_ptr<MainWindow> _main_window;
		std::unique_ptr<ShortcutManager> _shortcut_manager;
		std::unique_ptr<ProjectInfo> _project_info;
		std::unique_ptr<PreferencesDesc> _preferences_desc;
		std::unique_ptr<LiveSettings> _live_settings;

		Editor();
		Editor(const Editor&) = delete;
		Editor(Editor&&) = delete;

	public:
		FunctionalEvent<> OnPreferencesChanged;

		static Editor& Instance();
		void Init(imtk::os_window* window);
		void Terminate();
		void Tick();
		size_t GetFrame() const;

		void SetOSWindowSize(int width, int height);
		void SetOSWindowMaximized(bool maximized);
		void SetOSWindowFullScreen(bool fullscreen);
		bool IsOSWindowFullScreen() const;
		void RequestShutdown();

		static PreferencesDesc& GetPreferences();
		static LiveSettingsDesc& GetLiveSettings();

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
