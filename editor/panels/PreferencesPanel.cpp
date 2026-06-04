#include "PreferencesPanel.h"

#include "core/MainWindow.h"
#include "core/Errors.h"
#include "panels/PanelManager.h"

#include <imgui.h>

namespace oly::editor
{
	PreferencesPanel& PreferencesPanel::Instance()
	{
		if (auto panel = MainWindow::Instance().GetPanelManager().Get<PreferencesPanel>())
			return *panel;
		else
			BreakoutError::Throw("No instance of PreferencesPanel");
	}

	void PreferencesPanel::Init()
	{
		// TODO v8 load preferences
	}

	const char* PreferencesPanel::GetTitle() const
	{
		return "Preferences";
	}
	
	void PreferencesPanel::Draw()
	{
		if (auto window = DrawDockedWindow(ImGuiWindowFlags_None))
		{
			// TODO v8 use PreferencesDesc
		}
	}
}
