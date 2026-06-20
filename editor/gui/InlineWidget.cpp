#include "InlineWidget.h"

#include "gui/scopes/IDScope.h"

#include <imgui.h>

namespace oly::editor::gui
{
	// TODO v9.1 wrap all 

	void InlineWidget::Draw(InlineWidgetSettings settings)
	{
		gui::IDScope scope(this);

		ImVec2 size(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeightWithSpacing());

		if (ImGui::BeginChild("##InlineWidget", size))
		{
			if (ImGui::BeginTable("##Table", static_cast<int>(_components.size())))
			{
				ImGui::TableNextRow();

				for (const auto& draw_fn : _components)
				{
					ImGui::TableNextColumn();
					draw_fn();
				}

				ImGui::EndTable();
			}
		}

		ImGui::EndChild();
	}

	void InlineWidget::AddComponent(std::function<void()> draw_fn)
	{
		_components.push_back(std::move(draw_fn));
	}
}
