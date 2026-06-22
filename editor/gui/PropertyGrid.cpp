#include "PropertyGrid.h"

#include "core/editor/ResourceLoader.h"

#include "gui/graphics/Toolbar.h"
#include "gui/scopes/IDScope.h"
#include "gui/PropertyGroup.h"

#include <array>
#include <unordered_set>

namespace oly::editor::gui
{
	static std::string KEY_LABEL;

	static std::vector<WidgetComponent> VALUE_COMPONENTS;
	static DrawResult VALUE_DRAW_RESULT;
	
	static std::unordered_set<size_t> SUBROWS_TO_RESET;
	static std::unordered_set<size_t> ACTIVATED_RESET_SUBROWS;

	static bool DIRTY_GRID = false;

	void PropertyGrid::Key::SetLabel(const std::string_view label)
	{
		KEY_LABEL = label;
	}

	// TODO v9.1 Currently, PropertyGroup::Submit() is called in key cell draw and PropertyGroup::Clear() is called at end of SubmitRow(). Only clear on Form end, but still call Submit() for a row's properties. Instead of continuous vector of all properties, will need to divide PROPERTIES into data structure where each row has a list of properties, and each form has a list of properties + sublists. Can nest forms indefinitely.

	DrawResult PropertyGrid::Value::GetDrawResult()
	{
		return VALUE_DRAW_RESULT;
	}

	void PropertyGrid::Value::AddComponent(WidgetComponent component)
	{
		VALUE_COMPONENTS.push_back(std::move(component));
	}

	void PropertyGrid::Reset::Button(size_t subrow)
	{
		SUBROWS_TO_RESET.insert(subrow);
	}

	bool PropertyGrid::Reset::Activated(size_t subrow)
	{
		return ACTIVATED_RESET_SUBROWS.contains(subrow);
	}

	bool PropertyGrid::Reset::AnyActivated()
	{
		return !ACTIVATED_RESET_SUBROWS.empty();
	}

	static void DrawKeyCell()
	{
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(KEY_LABEL.c_str());
		KEY_LABEL.clear();
		PropertyGroup::Submit();
	}

	static float GetItemWidth(const float remaining_width, const size_t remaining_count)
	{
		return remaining_count > 0 ? (remaining_width - ImGui::GetStyle().ItemSpacing.x * (remaining_count - 1)) / remaining_count : remaining_width;
	}

	static void DrawValueCell()
	{
		ImGui::TableSetColumnIndex(1);

		VALUE_DRAW_RESULT = {};

		float remaining_width = ImGui::GetContentRegionAvail().x;
		for (size_t i = 0; i < VALUE_COMPONENTS.size(); ++i)
		{
			const float item_width = GetItemWidth(remaining_width, VALUE_COMPONENTS.size() - i);
			ImGui::SetNextItemWidth(item_width);
			
			const float start_x = ImGui::GetCursorPosX();
			VALUE_DRAW_RESULT |= VALUE_COMPONENTS[i].draw();
			if (i + 1 < VALUE_COMPONENTS.size())
				ImGui::SameLine();
			remaining_width -= ImGui::GetCursorPosX() - start_x;
		}

		VALUE_COMPONENTS.clear();
	}

	static void DrawResetCell()
	{
		ImGui::TableSetColumnIndex(2);

		ACTIVATED_RESET_SUBROWS.clear();
		size_t subrow = 0;
		while (!SUBROWS_TO_RESET.empty())
		{
			auto it = SUBROWS_TO_RESET.find(subrow);
			if (it != SUBROWS_TO_RESET.end())
			{
				SUBROWS_TO_RESET.erase(it);

				IDScope scope;
				scope.Push(subrow);
				if (Toolbar::DrawIconButton(IconResource::Revert, "Reset to default", "##Revert"))
					ACTIVATED_RESET_SUBROWS.insert(subrow);
			}
			else
				ImGui::NewLine();

			++subrow;
		}
	}

	void PropertyGrid::SubmitRow()
	{
		ImGui::TableNextRow();
		DrawResetCell();
		DrawValueCell();
		DrawKeyCell();
		DIRTY_GRID |= DirtyRow();
		PropertyGroup::Clear();
	}

	bool PropertyGrid::DirtyRow()
	{
		return Value::GetDrawResult().IsDirty() || Reset::AnyActivated();
	}

	void PropertyGrid::Clear()
	{
		KEY_LABEL.clear();

		VALUE_COMPONENTS.clear();
		VALUE_DRAW_RESULT = {};

		SUBROWS_TO_RESET.clear();
		ACTIVATED_RESET_SUBROWS.clear();

		DIRTY_GRID = false;

		PropertyGroup::Clear();
	}

	bool PropertyGrid::DirtyGrid()
	{
		return DIRTY_GRID;
	}

	bool PropertyGrid::BeginTable()
	{
		if (ImGui::BeginTable("", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
			return true;
		}
		else
			return false;
	}
}
