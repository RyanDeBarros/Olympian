#pragma once

#include <imgui.h>

#include <memory>

class PanelManager;

class MainWindow
{
	ImGuiID _dockspace_id = 0;
	std::unique_ptr<PanelManager> _panel_manager;

public:
	MainWindow();
	~MainWindow();

	void Init();
	void Draw();
};
