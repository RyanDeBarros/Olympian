#include "Editor.h"

#include "Logger.h"
#include "MainWindow.h"

Editor::Editor()
	: _logger(std::make_unique<Logger>()), _main_window(std::make_unique<MainWindow>())
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
