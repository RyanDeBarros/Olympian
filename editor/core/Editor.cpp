#include "Editor.h"

#include "core/Logger.h"
#include "core/MainWindow.h"
#include "core/ShortcutManager.h"
#include "core/ProjectInfo.h"

#include "documents/DocumentManager.h"

namespace oly::editor
{
	Editor::Editor()
		: _logger(std::make_unique<Logger>()),
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

	void Editor::Init()
	{
		// TODO v7 start editor with project select window.
		_project_info->Init("D:/Projects/Visual Studio/Olympian/Tester/");
	}

	void Editor::Tick()
	{
		_shortcut_manager->PollShortcuts();

		if (!_ui_initialized)
		{
			_main_window->Init();
			_ui_initialized = true;
		}

		_main_window->Draw();
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

	void Editor::OpenFile(const std::filesystem::path& path)
	{
		DocumentManager::Instance().OpenAsset(path);
	}
}
