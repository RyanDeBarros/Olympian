#include "Editor.h"

#include "core/Errors.h"
#include "core/windows/ProjectSelectWindow.h"
#include "core/windows/MainWindow.h"

#include "core/editor/LiveSettings.h"
#include "core/editor/Logger.h"
#include "core/editor/Notifier.h"
#include "core/editor/ProjectInfo.h"
#include "core/editor/ResourceLoader.h"
#include "core/editor/ShortcutManager.h"

#include "documents/DocumentManager.h"

#include "documents/FontFamilyDocument.h"
#include "documents/RasterFontDocument.h"
#include "documents/SignalDocument.h"
#include "documents/TilesetDocument.h"

#include "panels/AssetEditorPanel.h"
#include "panels/PreferencesPanel.h"

#include "desc/impl/PreferencesDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{	
	Editor::Editor() :
		_project_select_window(std::make_unique<ProjectSelectWindow>()),
		_logger(std::make_unique<Logger>()),
		_main_window(std::make_unique<MainWindow>()),
		_shortcut_manager(std::make_unique<ShortcutManager>()),
		_project_info(std::make_unique<ProjectInfo>())
	{
		_os_window = std::make_unique<imtk::os_window>(1, 1, "Olympian Editor");

		LoadAllIcons();

		imtk::post_window_init({
			.error_logger = [](std::string_view error) { BreakoutError::Log(error); },
			.reset_icon = Icon(IconResource::Revert),
			.key_encoder = [](imtk::key key) -> std::string { return detail::encode_key(key); },
			.key_decoder = [](std::string_view key) -> imtk::key { return detail::decode_key(key); }
		});

		glfwSetDropCallback(_os_window->get(), [](GLFWwindow* window, int count, const char** paths) {
			ShortcutManager::Instance().HandlePathDrop(count, paths);
		});

		glfwSetWindowCloseCallback(_os_window->get(), [](GLFWwindow* w) {
			glfwSetWindowShouldClose(w, GLFW_FALSE);
			Editor::instance().RequestShutdown();
		});

		_app_state = AppState::ProjectSelect;
		_project_select_window->Open();
	}

	Editor::~Editor() = default;

	imp::functional_event<>& Editor::OnPreferencesChanged()
	{
		return instance()._on_preferences_changed;
	}

	bool Editor::ShouldClose() const
	{
		return _os_window->should_close();
	}

	// TODO v9.4 handle breakout errors - handle at closest convenience, for example each document handles its own breakout errors, each panel does, etc. so that one breakout error doesn't cut the full frame short.
	void Editor::Tick()
	{
		_os_window->begin_frame();
		imtk::begin_frame();

		// TODO v9.4 likewise with breakout errors, use imtk::handle_error at each entry point (panel, document, etc.)
		imtk::handle_error([this]() {
			_shortcut_manager->PollShortcuts();

			switch (_app_state)
			{
			case AppState::ProjectSelect:
				_project_select_window->Draw();
				break;
			case AppState::Main:
				_main_window->Draw();
				break;
			}
		});

		imtk::end_frame();
		_os_window->end_frame();
	}
	
	void Editor::SetOSWindowSize(int width, int height)
	{
		instance()._os_window->set_size(width, height);
	}

	void Editor::SetOSWindowMaximized(bool maximized)
	{
		instance()._os_window->set_maximized(maximized);
	}

	void Editor::SetOSWindowFullScreen(bool fullscreen)
	{
		instance()._os_window->set_fullscreen(fullscreen);
	}

	bool Editor::IsOSWindowFullScreen()
	{
		return instance()._os_window->is_fullscreen();
	}

	void Editor::RequestShutdown()
	{
		if (instance()._app_state == AppState::Main)
		{
			if (!PreferencesPanel::Instance().RequestShutdown())
				return;

			if (!AssetEditorPanel::Instance().RequestShutdown())
				return;
		}

		if (instance()._live_settings)
			instance()._live_settings->Dump();

		glfwSetWindowShouldClose(instance()._os_window->get(), GLFW_TRUE);
	}

	PreferencesDesc& Editor::GetPreferences()
	{
		return *instance()._preferences_desc;
	}

	LiveSettingsDesc& Editor::GetLiveSettings()
	{
		return instance()._live_settings->desc;
	}

	AppState Editor::GetAppState()
	{
		return instance()._app_state;
	}

	ProjectSelectWindow& Editor::GetProjectSelectWindow()
	{
		return *instance()._project_select_window;
	}

	Logger& Editor::GetLogger()
	{
		return *instance()._logger;
	}

	MainWindow& Editor::GetMainWindow()
	{
		return *instance()._main_window;
	}

	ShortcutManager& Editor::GetShortcutManager()
	{
		return *instance()._shortcut_manager;
	}

	ProjectInfo& Editor::GetProjectInfo()
	{
		return *instance()._project_info;
	}

	void Editor::OpenProject(const std::filesystem::path& path)
	{
		instance()._app_state = AppState::Main;
		instance()._project_info->Init(path);
		instance()._main_window->Open();

		instance()._preferences_desc = std::make_unique<PreferencesDesc>();
		instance()._live_settings = std::make_unique<LiveSettings>();
		instance()._live_settings->Load();
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
			message += "asset meta version not supported"; // TODO v10 version mismatch handling
			break;

		case OpenAssetCode::UnsupportedExtension:
			message += "asset has unsupported file extension";
			break;

		case OpenAssetCode::DoesNotExist:
			message += "file does not exist";
			break;
		}

		Notifier::NotifyError(std::move(message));
	}

	bool Editor::InitNewAsset(detail::ResourcePath path, detail::Key meta_type)
	{
		return DocumentManager::Instance().InitNewAsset(path, meta_type);
	}

	bool Editor::ImportAsset(const detail::ResourcePath& source_asset)
	{
		return DocumentManager::Instance().ImportAsset(source_asset);
	}
}
