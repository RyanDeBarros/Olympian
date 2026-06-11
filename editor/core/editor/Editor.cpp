#include "Editor.h"

#include "core/windows/ProjectSelectWindow.h"
#include "core/editor/Logger.h"
#include "core/windows/MainWindow.h"
#include "core/editor/ShortcutManager.h"
#include "core/editor/ProjectInfo.h"
#include "core/editor/ResourceLoader.h"

#include "documents/DocumentManager.h"
#include "panels/AssetEditorPanel.h"

#include <imgui_impl_glfw.h>

namespace oly::editor
{
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

	void Editor::Init(GLFWwindow* window)
	{
		_os_window = window;
		ResourceLoader::LoadAll();
		_app_state = AppState::ProjectSelect;
		_project_select_window->Open();
	}

	void Editor::Tick()
	{
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
	}
	
	void Editor::SetOSWindowSize(int width, int height)
	{
		int x, y;
		int w, h;
		glfwGetWindowPos(_os_window, &x, &y);
		glfwGetWindowSize(_os_window, &w, &h);

		const float scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
		const int nw = static_cast<int>(width * scale);
		const int nh = static_cast<int>(height * scale);
		glfwSetWindowSize(_os_window, nw, nh);

		const int nx = x + (w - nw) / 2;
		const int ny = y + (h - nh) / 2;
		glfwSetWindowPos(_os_window, nx, ny);
	}

	void Editor::SetOSWindowMaximized(bool maximized)
	{
		if (maximized)
			glfwMaximizeWindow(_os_window);
		else
			glfwRestoreWindow(_os_window);
	}

	void Editor::SetOSWindowFullScreen(bool fullscreen)
	{
		if (fullscreen == _os_state.fullscreen)
			return;

		_os_state.fullscreen = fullscreen;
		if (fullscreen)
		{
			glfwGetWindowPos(_os_window, &_os_state.x, &_os_state.y);
			glfwGetWindowSize(_os_window, &_os_state.w, &_os_state.h);

			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(_os_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			glfwSetWindowMonitor(_os_window, nullptr, _os_state.x, _os_state.y, _os_state.w, _os_state.h, 0);
		}
	}

	bool Editor::IsOSWindowFullScreen() const
	{
		return _os_state.fullscreen;
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
	}

	void Editor::OpenFile(const std::filesystem::path& path)
	{
		OpenAssetCode code = DocumentManager::Instance().OpenAsset(path);
		if (code == OpenAssetCode::Success)
		{
			AssetEditorPanel::Instance().GainFocus();
			return;
		}

		Notification notif(LogLevel::Error, "cannot open " + path.generic_string() + ": ");
		switch (code)
		{
		case OpenAssetCode::UnsupportedAssetType:
			notif.message += "asset type not supported";
			break;
		case OpenAssetCode::UnsupportedAssetVersion:
			notif.message += "asset meta version not supported"; // TODO v9 version mismatch handling
			break;
		case OpenAssetCode::UnsupportedExtension:
			notif.message += "asset has unsupported file extension";
			break;
		case OpenAssetCode::DoesNotExist:
			notif.message += "file does not exist";
			break;
		}

		_main_window->PushNotification(std::move(notif));
	}
}
