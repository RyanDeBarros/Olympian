#pragma once

#include "assets/KeyDecl.h"
#include "assets/ResourcePath.h"

#include <imtk.hpp>

#include <imp/event.hpp>

#include <filesystem>

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
        : public imp::instance_guard<Editor>
    {
        std::unique_ptr<imtk::os_window> os_window_;
        AppState app_state_ = AppState::ProjectSelect;

        std::unique_ptr<ProjectSelectWindow> project_select_window_;

        std::unique_ptr<MainWindow> main_window_;
        std::unique_ptr<ShortcutManager> shortcut_manager_;
        std::unique_ptr<ProjectInfo> project_info_;
        std::unique_ptr<PreferencesDesc> preferences_desc_;
        std::unique_ptr<LiveSettings> live_settings_;
        imp::event<> on_preferences_changed_;

    public:
        Editor();
        ~Editor();

    public:
        static imp::event<>& OnPreferencesChanged();

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

        bool ShouldClose() const;
        void Tick();
    };
}
