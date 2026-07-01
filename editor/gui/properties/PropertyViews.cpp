#include "PropertyViews.h"

namespace oly::editor::prop
{
	static PropUID COMBO_UID = OLY_DECL_PROP_UID;

	struct ComboPropertyPayload
	{
		int index;
		LabelSpanRegistry::Handle names;
	};

	ComboPropertyView::ComboPropertyView(int& index, LabelSpanRegistry::Handle names)
		: index(index), names(names)
	{
	}

	RawPropertyPayload ComboPropertyView::Dump() const
	{
		return RawPropertyPayload::Make(ComboPropertyPayload{ .index = index, .names = names }, COMBO_UID);
	}

	bool ComboPropertyView::CanParse(const RawPropertyPayload& payload) const
	{
		if (payload.type == COMBO_UID)
		{
			auto& data = *reinterpret_cast<const ComboPropertyPayload*>(payload.data.data());
			if (data.names == names)
				return true;
			else
				return false;
		}
		else
			return false;
	}

	bool ComboPropertyView::TryParse(const RawPropertyPayload& payload) const
	{
		if (CanParse(payload))
		{
			auto& data = *reinterpret_cast<const ComboPropertyPayload*>(payload.data.data());
			if (index != data.index)
			{
				index = data.index;
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}
}
