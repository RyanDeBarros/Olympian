#include "TreeViewPanel.h"

#include <imgui.h>

namespace oly::editor
{
	const char* TreeViewPanel::GetTitle() const
	{
		return "Tree View";
	}

	void TreeViewPanel::Draw()
	{
		ImGui::Begin(GetTitle());
		// TODO v8
		ImGui::End();
	}
}
