#include "CollapsingSection.h"

#include "gui/scopes/StyleStack.h"

namespace oly::editor
{
	static int SECTION_BG_TOGGLE = 0;

	CollapsingSection::CollapsingSection(const char* label, bool start_open)
	{
		gui::StyleVar1D sv(ImGuiStyleVar_ChildBorderSize, 2.f);

		ImVec4 border_col = ImGui::GetStyle().Colors[ImGuiCol_Border];
		const float w = border_col.w;
		border_col *= 1.f + 0.35f * SECTION_BG_TOGGLE++;
		border_col.w = w;
		gui::StyleColor sc(ImGuiCol_Border, ImGui::GetColorU32(border_col));

		if (ImGui::BeginChild(label, ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders))
		{
			if (start_open)
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			_visible = ImGui::TreeNode(label);
		}
	}

	CollapsingSection::CollapsingSection(CollapsingSection&& other) noexcept
		: _visible(other._visible)
	{
		other._visible = false;
		other._valid = false;
	}

	CollapsingSection::~CollapsingSection()
	{
		if (_valid)
		{
			if (_visible)
				ImGui::TreePop();

			ImGui::EndChild();
			--SECTION_BG_TOGGLE;
		}
	}

	CollapsingSection::operator bool() const
	{
		return _visible;
	}
}
