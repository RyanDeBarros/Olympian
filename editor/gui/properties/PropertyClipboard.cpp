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

	RawPropertyPayload RawPropertyPayload::Make(const void* data, size_t size, PropUID type)
	{
		RawPropertyPayload payload;
		payload.data.resize(size);
		std::memcpy(payload.data.data(), data, size);
		payload.type = type;
		return payload;
	}

	static std::vector<RawPropertyPayload> CLIPBOARD;

	void PropertyClipboard::Clear()
	{
		CLIPBOARD.clear();
	}

	void PropertyClipboard::Store(const std::span<IPropertyView*> props)
	{
		CLIPBOARD.clear();
		CLIPBOARD.reserve(props.size());
		for (const auto& prop : props)
			CLIPBOARD.push_back(prop->Dump());
	}

	bool PropertyClipboard::CanPaste(const std::span<IPropertyView*> props)
	{
		if (!CLIPBOARD.empty())
		{
			if (props.size() == CLIPBOARD.size())
			{
				for (size_t i = 0; i < CLIPBOARD.size(); ++i)
				{
					if (!props[i]->CanParse(CLIPBOARD[i]))
						return false;
				}

				return true;
			}
			else
				return false;
		}
		else
			return false;
	}

	bool PropertyClipboard::TryPaste(const std::span<IPropertyView*> props)
	{
		if (CanPaste(props))
		{
			bool dirty = false;
			for (size_t i = 0; i < CLIPBOARD.size(); ++i)
				dirty |= props[i]->TryParse(CLIPBOARD[i]);
			return dirty;
		}
		else
			return false;
	}

	bool PropertyClipboard::ContextMenuItems(const std::span<IPropertyView*> props)
	{
		if (ImGui::MenuItem("Copy"))
			Store(props);

		if (auto disabled = DisabledSection(!CanPaste(props)))
		{
			if (ImGui::MenuItem("Paste"))
				return TryPaste(props);
		}

		return false;
	}
}
