#pragma once

#include <memory>

class Logger;
class MainWindow;

class Editor
{
	bool _ui_initialized = false;
	std::unique_ptr<Logger> _logger;
	std::unique_ptr<MainWindow> _main_window;

	Editor();
	Editor(const Editor&) = delete;
	Editor(Editor&&) = delete;

public:
	static Editor& Instance();
	void Init();
	void Draw();

	Logger& GetLogger();
};
