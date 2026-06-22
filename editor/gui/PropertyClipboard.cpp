#include "PropertyClipboard.h"

#include "core/editor/UID.h"

#include <string.h>

namespace oly::editor
{
	RawPropertyPayload::RawPropertyPayload()
		: type(UID::None)
	{
	}

	RawPropertyPayload RawPropertyPayload::Make(const void* stack_payload, size_t size, UID type)
	{
		RawPropertyPayload payload;
		payload.data.resize(size);
		memcpy_s(payload.data.data(), payload.data.size(), stack_payload, size);
		payload.type = type;
		return payload;
	}

	bool RawPropertyPayload::Empty() const
	{
		return type == UID::None || data.empty();
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
}
