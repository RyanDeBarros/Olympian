#include "PropertyGroup.h"

#include "core/Errors.h"
#include "gui/scopes/IDScope.h"

#include <imgui.h>

#include <string>
#include <variant>

namespace oly::editor
{
	static size_t CONTEXT_MENU_ID_COUNTER = 0;

	void PropertyGroup::Begin()
	{
		CONTEXT_MENU_ID_COUNTER = 0;
	}

	void PropertyGroup::End()
	{
		// NOP
	}

	static bool BeginContextMenu()
	{
		return ImGui::BeginPopupContextItem(("##" + std::to_string(CONTEXT_MENU_ID_COUNTER++)).c_str());
	}
	
	bool PropertyGroup::CheckValue(const IPropertyView& prop)
	{
		bool dirty = false;
		if (BeginContextMenu())
		{
			dirty = PropertyClipboard::ContextMenuItems(prop);
			ImGui::EndPopup();
		}
		return dirty;
	}

	bool PropertyGroup::CheckRow(const PropertyRow& props)
	{
		bool dirty = false;
		if (BeginContextMenu())
		{
			dirty = PropertyClipboard::ContextMenuItems(props);
			ImGui::EndPopup();
		}
		return dirty;
	}

	bool PropertyGroup::CheckHeader(const PropertyPageGenerator& generator)
	{
		bool dirty = false;
		if (BeginContextMenu())
		{
			dirty = PropertyClipboard::ContextMenuItems(generator());
			ImGui::EndPopup();
		}
		return dirty;
	}
}
