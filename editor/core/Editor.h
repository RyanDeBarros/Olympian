#pragma once

#include <filesystem>
#include <memory>

class Logger;
class MainWindow;
class ShortcutManager;

class Editor
{
	bool _ui_initialized = false;
	std::unique_ptr<Logger> _logger;
	std::unique_ptr<MainWindow> _main_window;
	std::unique_ptr<ShortcutManager> _shortcut_manager;

	Editor();
	Editor(const Editor&) = delete;
	Editor(Editor&&) = delete;

public:
	static Editor& Instance();
	void Init();
	void Draw();

	Logger& GetLogger();
	MainWindow& GetMainWindow();
	ShortcutManager& GetShortcutManager();

	void OpenFile(const std::filesystem::path& path);
};
