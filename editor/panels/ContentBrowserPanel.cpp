#include "ContentBrowserPanel.h"

#include "core/windows/MainWindow.h"
#include "core/Errors.h"
#include "panels/PanelManager.h"

#include <imgui.h>

namespace oly::editor
{
	ContentBrowserPanel& ContentBrowserPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<ContentBrowserPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of ContentBrowserPanel");
	}

	void ContentBrowserPanel::Init()
	{
		// NOP
	}

	const char* ContentBrowserPanel::GetTitle() const
	{
		return "Content Browser";
	}

	void ContentBrowserPanel::Draw()
	{
		auto window = DrawDockedWindow(ImGuiWindowFlags_None);
		if (window.IsVisible())
		{
			// TODO v9.2
		}
	}
}
