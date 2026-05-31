#include "Editor.h"

#include "core/Logger.h"
#include "core/MainWindow.h"
#include "core/ShortcutManager.h"

#include "documents/DocumentManager.h"

Editor::Editor()
	: _logger(std::make_unique<Logger>()),
	_main_window(std::make_unique<MainWindow>()),
	_shortcut_manager(std::make_unique<ShortcutManager>())
{
}

Editor& Editor::Instance()
{
	static Editor editor;
	return editor;
}

void Editor::Init()
{
}

void Editor::Draw()
{
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

void Editor::OpenFile(const std::filesystem::path& path)
{
	DocumentManager::Instance().OpenAsset(path);
}
