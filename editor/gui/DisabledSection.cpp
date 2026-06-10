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
		if (_alive)
			ImGui::EndDisabled();
	}

	DisabledSection::DisabledSection(DisabledSection&& o) noexcept
		: _alive(o._alive)
	{
		o._alive = false;
	}

	DisabledSection::operator bool() const
	{
		return true;
	}
}
