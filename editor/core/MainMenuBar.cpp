#include "MainMenuBar.h"

#include <imgui.h>

namespace oly::editor
{
	void MainMenuBar::Init()
	{
		// NOP
	}

	void MainMenuBar::Draw()
	{
		if (ImGui::BeginMainMenuBar())
		{
			DrawWindowMenu();
			ImGui::EndMainMenuBar();
		}

		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize * 0.5f, ImGuiCond_FirstUseEver);
	}

	void MainMenuBar::DrawWindowMenu()
	{
		if (ImGui::BeginMenu("Window"))
		{
			// TODO v8 items to open each panel

			ImGui::EndMenu();
		}
	}
}
