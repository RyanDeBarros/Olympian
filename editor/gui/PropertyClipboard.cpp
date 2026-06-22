#include "PropertyClipboard.h"

#include "gui/scopes/DisabledSection.h"

#include <imgui.h>

#include <cstring>

namespace oly::editor
{
	namespace prop
	{
		PropUID _prop_uid_counter = 0;

		const PropUID NULL_UID = OLY_DECL_PROP_UID;
	}

	RawPropertyPayload::RawPropertyPayload()
		: type(prop::NULL_UID)
	{
	}

	RawPropertyPayload RawPropertyPayload::Make(const void* stack_payload, size_t size, PropUID type)
	{
		RawPropertyPayload payload;
		payload.data.resize(size);
		std::memcpy(payload.data.data(), stack_payload, size);
		payload.type = type;
		return payload;
	}

	bool RawPropertyPayload::Empty() const
	{
		return type == prop::NULL_UID || data.empty();
	}

	static RawPropertyPayload CLIPBOARD;

	void PropertyClipboard::Clear()
	{
		CLIPBOARD = {};
	}

	void PropertyClipboard::Store(const IPropertyView& prop)
	{
		CLIPBOARD = prop.Dump();
	}

	bool PropertyClipboard::CanPaste(const IPropertyView& prop)
	{
		if (!CLIPBOARD.Empty())
			return prop.CanParse(CLIPBOARD);
		else
			return false;
	}

	bool PropertyClipboard::TryPaste(IPropertyView& prop)
	{
		if (!CLIPBOARD.Empty())
			return prop.TryParse(CLIPBOARD);
		else
			return false;
	}

	bool PropertyClipboard::ContextMenuItems(IPropertyView& prop)
	{
		if (ImGui::MenuItem("Copy"))
			Store(prop);

		if (auto disabled = DisabledSection(!CanPaste(prop)))
		{
			if (ImGui::MenuItem("Paste"))
				return TryPaste(prop);
		}

		return false; // TODO v9.1 don't use bool to indicate document should MarkDirty(), use some wrapper to make it explicitly referring to a 'changed' event
	}
}
