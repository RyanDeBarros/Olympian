#include "PropertyGroup.h"

#include "gui/scopes/IDScope.h"

#include <imgui.h>

namespace oly::editor
{
	static std::vector<std::unique_ptr<IPropertyView>> PROPERTIES;

	void PropertyGroup::Clear()
	{
		PROPERTIES.clear();
	}

	static void ContextMenu(std::span<IPropertyView*> properties, bool& opened, bool& dirty)
	{
		opened = false;
		dirty = false;

		if (ImGui::BeginPopupContextItem("##"))
		{
			opened = true;
			dirty = PropertyClipboard::ContextMenuItems(properties);
			ImGui::EndPopup();
		}
	}

	bool PropertyGroup::Append(std::unique_ptr<IPropertyView>&& prop)
	{
		bool opened, dirty;
		IPropertyView* p = prop.get();
		gui::IDScope scope(prop.get());
		ContextMenu(std::span<IPropertyView*>(&p, 1), opened, dirty);

		if (opened)
			PROPERTIES.clear();
		else
			PROPERTIES.push_back(std::move(prop));

		return dirty;
	}

	bool PropertyGroup::Submit()
	{
		std::vector<IPropertyView*> properties;
		properties.reserve(PROPERTIES.size());
		for (auto& prop : PROPERTIES)
			properties.push_back(prop.get());

		bool opened, dirty;
		ContextMenu(properties, opened, dirty);
		return dirty;
	}
}
