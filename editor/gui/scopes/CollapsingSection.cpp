#include "CollapsingSection.h"

#include "gui/properties/PropertyGroup.h"

#include <imgui.h>

namespace oly::editor
{
	CollapsingSection::CollapsingSection(const char* label, bool start_open)
	{
		if (start_open)
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		_visible = ImGui::TreeNode(label);
	}

	CollapsingSection::CollapsingSection(CollapsingSection&& other) noexcept
		: _visible(other._visible)
	{
		other._visible = false;
	}

	CollapsingSection::~CollapsingSection()
	{
		if (_visible)
		{
			ImGui::TreePop();
			PropertyGroup::CheckHeader();
		}
	}

	CollapsingSection::operator bool() const
	{
		return _visible;
	}
}
