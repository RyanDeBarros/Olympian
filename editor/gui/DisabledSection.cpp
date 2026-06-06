#include "DisabledSection.h"

#include <imgui.h>

namespace oly::editor
{
	DisabledSection::DisabledSection(bool disabled)
	{
		ImGui::BeginDisabled(disabled);
	}

	DisabledSection::~DisabledSection()
	{
		ImGui::EndDisabled();
	}

	DisabledSection::operator bool() const
	{
		return true;
	}
}
