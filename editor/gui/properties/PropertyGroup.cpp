#include "PropertyGroup.h"

#include "core/Errors.h"

#include <imtk.hpp>

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

	static imtk::context_menu BeginContextMenu()
	{
		return imtk::context_menu::item("##" + std::to_string(CONTEXT_MENU_ID_COUNTER++));
	}
	
	bool PropertyGroup::CheckValue(const IPropertyView& prop)
	{
		bool dirty = false;
		if (auto _ = BeginContextMenu())
			dirty = PropertyClipboard::ContextMenuItems(prop);
		return dirty;
	}

	bool PropertyGroup::CheckRow(const PropertyRow& props)
	{
		bool dirty = false;
		if (auto _ = BeginContextMenu())
			dirty = PropertyClipboard::ContextMenuItems(props);
		return dirty;
	}

	bool PropertyGroup::CheckHeader(const PropertyPageGenerator& generator)
	{
		bool dirty = false;
		if (auto _ = BeginContextMenu())
			dirty = PropertyClipboard::ContextMenuItems(generator());
		return dirty;
	}
}
