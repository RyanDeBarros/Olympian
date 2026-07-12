#include "Editor.h"

#include "core/windows/ProjectSelectWindow.h"
#include "core/windows/MainWindow.h"

#include "core/editor/LiveSettings.h"
#include "core/editor/Logger.h"
#include "core/editor/Notifier.h"
#include "core/editor/ProjectInfo.h"
#include "core/editor/ResourceLoader.h"
#include "core/editor/ShortcutManager.h"

#include "gui/graphics/Texture.h"

#include "documents/DocumentManager.h"

#include "panels/AssetEditorPanel.h"
#include "panels/PreferencesPanel.h"

#include "desc/impl/PreferencesDesc.h"

namespace oly::editor
{
	static size_t FRAME_COUNTER = 0;

	Editor::Editor() :
		_project_select_window(std::make_unique<ProjectSelectWindow>()),
		_logger(std::make_unique<Logger>()),
		_main_window(std::make_unique<MainWindow>()),
		_shortcut_manager(std::make_unique<ShortcutManager>()),
		_project_info(std::make_unique<ProjectInfo>())
	{
	}

	Editor& Editor::Instance()
	{
		static Editor editor;
		return editor;
	}

	void Editor::Init(imtk::os_window* window)
	{
		_os_window = window;

		glfwSetDropCallback(_os_window->get(), [](GLFWwindow* window, int count, const char** paths) {
			ShortcutManager::Instance().HandlePathDrop(count, paths);
		});

		glfwSetWindowCloseCallback(_os_window->get(), [](GLFWwindow* w) {
			glfwSetWindowShouldClose(w, GLFW_FALSE);
			Editor::Instance().RequestShutdown();
		});

		ResourceLoader::LoadAll();
		_app_state = AppState::ProjectSelect;
		_project_select_window->Open();
	}

	void Editor::Terminate()
	{
		_project_select_window.reset();
		_logger.reset();
		_main_window.reset();
		_shortcut_manager.reset();
		_project_info.reset();
	}

	void Editor::Tick()
	{
		_shortcut_manager->PollShortcuts();
		Texture::Update();

		switch (_app_state)
		{
		case AppState::ProjectSelect:
			_project_select_window->Draw();
			break;
		case AppState::Main:
			_main_window->Draw();
			break;
		}

		++FRAME_COUNTER;
	}

	size_t Editor::GetFrame() const
	{
		return FRAME_COUNTER;
	}
	
	void Editor::SetOSWindowSize(int width, int height)
	{
		_os_window->set_size(width, height);
	}

	void Editor::SetOSWindowMaximized(bool maximized)
	{
		_os_window->set_maximized(maximized);
	}

	void Editor::SetOSWindowFullScreen(bool fullscreen)
	{
		_os_window->set_fullscreen(fullscreen);
	}

	bool Editor::IsOSWindowFullScreen() const
	{
		return _os_window->is_fullscreen();
	}

	void Editor::RequestShutdown()
	{
		if (_app_state == AppState::Main)
		{
			if (!PreferencesPanel::Instance().RequestShutdown())
				return;

			if (!AssetEditorPanel::Instance().RequestShutdown())
				return;
		}

		if (_live_settings)
			_live_settings->Dump();

		glfwSetWindowShouldClose(_os_window->get(), GLFW_TRUE);
	}

	PreferencesDesc& Editor::GetPreferences()
	{
		return *Instance()._preferences_desc;
	}

	LiveSettingsDesc& Editor::GetLiveSettings()
	{
		return Instance()._live_settings->desc;
	}

	AppState Editor::GetAppState() const
	{
		return _app_state;
	}

	ProjectSelectWindow& Editor::GetProjectSelectWindow()
	{
		return *_project_select_window;
	}

	Logger& Editor::GetLogger()
	{
		return *_logger;
	}

	MainWindow& Editor::GetMainWindow()
	{
		return *_main_window;
	}

	ShortcutManager& Editor::GetShortcutManager()
	{
		return *_shortcut_manager;
	}

	ProjectInfo& Editor::GetProjectInfo()
	{
		return *_project_info;
	}

	void Editor::OpenProject(const std::filesystem::path& path)
	{
		_app_state = AppState::Main;
		_project_info->Init(path);
		_main_window->Open();

		_preferences_desc = std::make_unique<PreferencesDesc>();
		_live_settings = std::make_unique<LiveSettings>();
		_live_settings->Load();
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
}
