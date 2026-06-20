#include "InlineWidget.h"

#include "gui/scopes/IDScope.h"

#include <imgui.h>

namespace oly::editor::gui
{
	DrawResult InlineWidget::Draw()
	{
		DrawResult result;
		gui::IDScope scope(this);

		if (ImGui::BeginChild("##InlineWidget", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeightWithSpacing())))
		{
			int columns = static_cast<int>(_components.size());
			if (ImGui::BeginTable("##Table", columns, ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_NoHostExtendX))
			{
				ImGui::TableNextRow();

				for (const auto& component : _components)
				{
					ImGui::TableNextColumn();
					result |= component.draw();
				}

				ImGui::EndTable();
			}
		}

		ImGui::EndChild();
		return result;
	}

	void InlineWidget::AddComponent(WidgetComponent component)
	{
		_components.push_back(std::move(component));
	}
}
