#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace oly::editor
{
	const char* ContentBrowserPanel::GetTitle() const
	{
		return "Content Browser";
	}

	void ContentBrowserPanel::Draw()
	{
		ImGui::Begin(GetTitle());
		// TODO v8
		ImGui::End();
	}
}
