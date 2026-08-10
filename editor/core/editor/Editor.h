#pragma once

#include "util/FunctionalEvent.h"

#include "assets/KeyDecl.h"
#include "assets/ResourcePath.h"

#include <imtk.hpp>

// TODO v9.3 remove once fio stuff is added to imtk
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

	class Editor : public imtk::instance_guard<Editor>
	{
		std::unique_ptr<imtk::os_window> _os_window;
		AppState _app_state = AppState::ProjectSelect;

		std::unique_ptr<ProjectSelectWindow> _project_select_window;

		std::unique_ptr<Logger> _logger;
		std::unique_ptr<MainWindow> _main_window;
		std::unique_ptr<ShortcutManager> _shortcut_manager;
		std::unique_ptr<ProjectInfo> _project_info;
		std::unique_ptr<PreferencesDesc> _preferences_desc;
		std::unique_ptr<LiveSettings> _live_settings;
		FunctionalEvent<> _on_preferences_changed;

	public:
		Editor();
		~Editor();

		static FunctionalEvent<>& OnPreferencesChanged();

		bool ShouldClose() const;
		void Tick();

		static void SetOSWindowSize(int width, int height);
		static void SetOSWindowMaximized(bool maximized);
		static void SetOSWindowFullScreen(bool fullscreen);
		static bool IsOSWindowFullScreen();
		static void RequestShutdown();

		static PreferencesDesc& GetPreferences();
		static LiveSettingsDesc& GetLiveSettings();

		static AppState GetAppState();
		static ProjectSelectWindow& GetProjectSelectWindow();
		static Logger& GetLogger();
		static MainWindow& GetMainWindow();
		static ShortcutManager& GetShortcutManager();
		static ProjectInfo& GetProjectInfo();

		static void OpenProject(const std::filesystem::path& path);
		static void OpenFile(const std::filesystem::path& path);
		static bool InitNewAsset(detail::ResourcePath path, detail::Key meta_type);
		static bool ImportAsset(const detail::ResourcePath& source_asset);
	};
}
