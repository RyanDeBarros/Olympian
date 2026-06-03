#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace oly::editor
{
	void ContentBrowserPanel::Init()
	{
		// TODO v8
	}

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
