#include "Editor.h"

#include "core/editor/LiveSettings.h"
#include "core/editor/ProjectInfo.h"
#include "core/editor/ResourceLoader.h"
#include "core/editor/ShortcutManager.h"

#include "core/windows/MainWindow.h"
#include "core/windows/ProjectSelectWindow.h"

#include "documents/DocumentManager.h"
#include "documents/FontFamilyDocument.h"
#include "documents/RasterFontDocument.h"
#include "documents/SignalDocument.h"
#include "documents/TilesetDocument.h"

#include "panels/AssetEditorPanel.h"
#include "panels/PreferencesPanel.h"

#include "desc/PreferencesDesc.h"

#include "assets/TranslateKey.h"

#include "definitions/Keys.h"

namespace oly::editor
{
    Editor::Editor()
        : project_select_window_(std::make_unique<ProjectSelectWindow>())
        , main_window_(std::make_unique<MainWindow>())
        , shortcut_manager_(std::make_unique<ShortcutManager>())
        , project_info_(std::make_unique<ProjectInfo>())
    {
        os_window_ = std::make_unique<imtk::os_window>(1, 1, "Olympian Editor");

        LoadAllIcons();

        imtk::post_window_init(
            {
                .reset_icon = Icon(IconResource::Revert),
                .key_encoder = [](imtk::key key) -> std::string { return detail::encode_key(key); },
                .key_decoder = [](std::string_view key) -> imtk::key { return detail::decode_key(key); },
                .dynamic_lists = {
                    .drag_icon = Icon(IconResource::Handle),
                    .create_icon = Icon(IconResource::Plus),
                    .delete_icon = Icon(IconResource::Minus),
                    .clear_icon = Icon(IconResource::Close),
                },
                .list_indexers = {
                    .create_icon = Icon(IconResource::Plus),
                    .delete_icon = Icon(IconResource::Minus),
                    .clear_icon = Icon(IconResource::Close),
                }
            });

        glfwSetDropCallback(
            os_window_->get(),
            [](GLFWwindow* window, int count, const char** paths) {
                ShortcutManager::Instance().HandlePathDrop(count, paths);
            });

        glfwSetWindowCloseCallback(
            os_window_->get(),
            [](GLFWwindow* w) {
                glfwSetWindowShouldClose(w, GLFW_FALSE);
                Editor::instance().RequestShutdown();
            });

        app_state_ = AppState::ProjectSelect;
        project_select_window_->Open();
    }

    Editor::~Editor() = default;

    imp::event<>& Editor::OnPreferencesChanged()
    {
        return instance().on_preferences_changed_;
    }

    void Editor::SetOSWindowSize(int width, int height)
    {
        instance().os_window_->set_size(width, height);
    }

    void Editor::SetOSWindowMaximized(bool maximized)
    {
        instance().os_window_->set_maximized(maximized);
    }

    void Editor::SetOSWindowFullScreen(bool fullscreen)
    {
        instance().os_window_->set_fullscreen(fullscreen);
    }

    bool Editor::IsOSWindowFullScreen()
    {
        return instance().os_window_->is_fullscreen();
    }

    void Editor::RequestShutdown()
    {
        if (instance().app_state_ == AppState::Main)
        {
            if (!PreferencesPanel::Instance().RequestShutdown())
                return;

            if (!AssetEditorPanel::Instance().RequestShutdown())
                return;
        }

        if (instance().live_settings_)
            instance().live_settings_->Dump();

        glfwSetWindowShouldClose(instance().os_window_->get(), GLFW_TRUE);
    }

    PreferencesDesc& Editor::GetPreferences()
    {
        return *instance().preferences_desc_;
    }

    LiveSettingsDesc& Editor::GetLiveSettings()
    {
        return instance().live_settings_->desc;
    }

    AppState Editor::GetAppState()
    {
        return instance().app_state_;
    }

    ProjectSelectWindow& Editor::GetProjectSelectWindow()
    {
        return *instance().project_select_window_;
    }

    MainWindow& Editor::GetMainWindow()
    {
        return *instance().main_window_;
    }

    ShortcutManager& Editor::GetShortcutManager()
    {
        return *instance().shortcut_manager_;
    }

    ProjectInfo& Editor::GetProjectInfo()
    {
        return *instance().project_info_;
    }

    void Editor::OpenProject(const std::filesystem::path& path)
    {
        instance().app_state_ = AppState::Main;
        instance().project_info_->Init(path);
        instance().main_window_->Open();

        instance().preferences_desc_ = std::make_unique<PreferencesDesc>();
        instance().live_settings_ = std::make_unique<LiveSettings>();
        instance().live_settings_->Load();
    }

    void Editor::OpenFile(const std::filesystem::path& path)
    {
        OpenAssetCode code = DocumentManager::Instance().OpenAsset(path);
        if (code == OpenAssetCode::Success)
        {
            AssetEditorPanel::Instance().Open();
            AssetEditorPanel::Instance().GainFocus();
            return;
        }

        std::string message = "cannot open " + path.generic_string() + ": ";
        switch (code)
        {
        case OpenAssetCode::UnsupportedAssetType:
            message += "asset type not supported";
            break;

        case OpenAssetCode::UnsupportedAssetVersion:
            message += "asset meta version not supported";
            break;

        case OpenAssetCode::UnsupportedExtension:
            message += "asset has unsupported file extension";
            break;

        case OpenAssetCode::DoesNotExist:
            message += "file does not exist";
            break;
        }

        imtk::notify_error(std::move(message));
    }

    bool Editor::InitNewAsset(detail::ResourcePath path, detail::Key meta_type)
    {
        return DocumentManager::Instance().InitNewAsset(path, meta_type);
    }

    bool Editor::ImportAsset(const detail::ResourcePath& source_asset)
    {
        return DocumentManager::Instance().ImportAsset(source_asset);
    }

    bool Editor::ShouldClose() const
    {
        return os_window_->should_close();
    }

    void Editor::Tick()
    {
        os_window_->begin_frame();
        imtk::begin_frame();

        imtk::handle_errors(
            [this]() {
            shortcut_manager_->PollShortcuts();

            switch (app_state_)
            {
            case AppState::ProjectSelect:
                project_select_window_->Draw();
                break;
            case AppState::Main:
                main_window_->Draw();
                break;
            }
        });

        imtk::end_frame();
        os_window_->end_frame();
    }
}
