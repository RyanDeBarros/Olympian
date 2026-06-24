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
	
	// TODO v9.1 Remove Indent. If a tree node is collapsed, its values should still be copied even though the draw functions never execute -> so replace Append() with CheckValue(datapath), and pass datapaths to CheckRow()/CheckHeader() -> no data structure should be stored here.

	//PropertyGroup::Subgroup::Subgroup()
	//{
	//	if (ACTIVE_PROPERTY_NODE)
	//		ACTIVE_PROPERTY_NODE = ACTIVE_PROPERTY_NODE->AppendNode();
	//	else
	//		BreakoutError::Throw("PropertyGroup::Subgroup::Subgroup(): active property node is null");
	//}

	//PropertyGroup::Subgroup::~Subgroup()
	//{
	//	if (ACTIVE_PROPERTY_NODE)
	//		ACTIVE_PROPERTY_NODE = ACTIVE_PROPERTY_NODE->parent;
	//	else
	//		BreakoutError::Throw("PropertyGroup::Subgroup::~Subgroup(): active property node is null");
	//}

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

	bool PropertyGroup::CheckHeader(const PropertyPage& props)
	{
		bool dirty = false;
		if (BeginContextMenu())
		{
			dirty = PropertyClipboard::ContextMenuItems(props);
			ImGui::EndPopup();
		}
		return dirty;
	}
}
