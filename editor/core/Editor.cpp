#include "Editor.h"

#include "core/ProjectSelectWindow.h"
#include "core/Logger.h"
#include "core/MainWindow.h"
#include "core/ShortcutManager.h"
#include "core/ProjectInfo.h"

#include "documents/DocumentManager.h"

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
		DocumentManager::Instance().OpenAsset(path);
	}
}
