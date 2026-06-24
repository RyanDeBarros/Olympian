#include "PreferencesPanel.h"

#include "core/Errors.h"
#include "core/windows/MainWindow.h"
#include "panels/PanelManager.h"

#include "definitions/Keys.h"

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
		doc.Init();
	}

	const char* PreferencesPanel::GetTitle() const
	{
		return "Preferences";
	}

	void PreferencesPanel::Draw()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
		if (doc.IsDirty())
			flags |= ImGuiWindowFlags_UnsavedDocument;

		auto window = DrawDockedWindow(flags);
		if (window.RequestsClose() && doc.IsDirty())
		{
			Open();
			ImGui::SetWindowFocus();
		}

		if (window.IsVisible())
		{
			if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
				doc.Dump();

			doc.DrawMenuBar();
			doc.Draw();
		}
	}

	const PreferencesDesc& PreferencesPanel::GetSavedDesc() const
	{
		return doc.GetSavedDesc();
	}
}
