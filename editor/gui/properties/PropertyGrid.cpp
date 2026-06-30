#include "PropertyGrid.h"

#include "core/editor/ResourceLoader.h"
#include "core/Errors.h"

#include "gui/InlineWidget.h"
#include "gui/graphics/Toolbar.h"
#include "gui/scopes/IDScope.h"

#include <array>
#include <unordered_set>

namespace oly::editor::gui
{
	static PropertyGrid* GRID_INSTANCE = nullptr;

	static std::string KEY_LABEL;
	static DrawResult KEY_DRAW_RESULT;

	static std::vector<WidgetComponent> VALUE_COMPONENTS;
	static PropertyRow VALUE_PROPERTIES;
	static DrawResult VALUE_DRAW_RESULT;

	static DrawResult FULL_DRAW_RESULT;
	
	static std::unordered_set<size_t> SUBROWS_TO_RESET;
	static std::unordered_set<size_t> ACTIVATED_RESET_SUBROWS;

	static bool DIRTY_GRID = false;

	static void ClearRow()
	{
		VALUE_COMPONENTS.clear();
		VALUE_PROPERTIES.list.clear();

		SUBROWS_TO_RESET.clear();
	}

	PropertyGrid::PropertyGrid()
	{
		if (GRID_INSTANCE)
			BreakoutError::Throw("PropertyGrid::Instance() called while a property grid instance already exists");

		GRID_INSTANCE = this;

		ClearRow();

		KEY_DRAW_RESULT = {};
		VALUE_DRAW_RESULT = {};
		FULL_DRAW_RESULT = {};
		DIRTY_GRID = false;
		KEY_LABEL.clear();
		ACTIVATED_RESET_SUBROWS.clear();

		PropertyGroup::Begin();
	}

	PropertyGrid::PropertyGrid(PropertyGrid&& o) noexcept
	{
		if (GRID_INSTANCE == &o)
			GRID_INSTANCE = this;
	}

	PropertyGrid::~PropertyGrid()
	{
		if (GRID_INSTANCE == this)
		{
			GRID_INSTANCE = nullptr;

			PropertyGroup::End();
		}
	}

	DrawResult PropertyGrid::Key::GetDrawResult()
	{
		return KEY_DRAW_RESULT;
	}

	void PropertyGrid::Key::SetLabel(const std::string_view label)
	{
		KEY_LABEL = label;
	}

	DrawResult PropertyGrid::Value::GetDrawResult()
	{
		return VALUE_DRAW_RESULT;
	}

	void PropertyGrid::Value::AddComponent(WidgetComponent component)
	{
		VALUE_COMPONENTS.push_back(std::move(component));
	}

	bool PropertyGrid::Value::CheckProperty(std::unique_ptr<IPropertyView>&& prop)
	{
		bool immediate = PropertyGroup::CheckValue(*prop);
		DIRTY_GRID |= immediate;
		VALUE_PROPERTIES.list.push_back(std::move(prop));
		return immediate;
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

		KEY_DRAW_RESULT = DrawResult().Query();
		KEY_DRAW_RESULT |= PropertyGroup::CheckRow(VALUE_PROPERTIES);
	}

	static void DrawValueCell()
	{
		ImGui::TableSetColumnIndex(1);
		VALUE_DRAW_RESULT = {};
		VALUE_DRAW_RESULT |= InlineWidget(VALUE_COMPONENTS);
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

	DrawResult PropertyGrid::GetFullDrawResult()
	{
		return FULL_DRAW_RESULT;
	}

	void PropertyGrid::SubmitRow()
	{
		ImGui::TableNextRow();
		DrawResetCell();
		DrawValueCell();
		DrawKeyCell();
		FULL_DRAW_RESULT = VALUE_DRAW_RESULT | KEY_DRAW_RESULT;
		DIRTY_GRID |= DirtyRow();
		ClearRow();
	}

	bool PropertyGrid::DirtyRow()
	{
		return Value::GetDrawResult().IsDirty() || Reset::AnyActivated();
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
