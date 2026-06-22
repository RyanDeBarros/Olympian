#include "PropertyViews.h"

namespace oly::editor::prop
{
	static PropUID COMBO_UID = OLY_DECL_PROP_UID;

	struct ComboPropertyPayload
	{
		int index;
		const char** names;
		size_t count;
	};

	// TODO v9.1 use string database to guarantee that names pointer is always valid and pointers can be directly compared without checking element-wise. Also store count so we can only pass the handle here.
	ComboPropertyView::ComboPropertyView(int& index, const char** names, size_t count)
		: index(index), names(names), count(count)
	{
	}

	RawPropertyPayload ComboPropertyView::Dump() const
	{
		return RawPropertyPayload::Make(ComboPropertyPayload{ .index = index, .names = names, .count = count }, COMBO_UID);
	}

	bool ComboPropertyView::CanParse(const RawPropertyPayload& payload) const
	{
		if (payload.type == COMBO_UID)
		{
			auto& data = *reinterpret_cast<const ComboPropertyPayload*>(payload.data.data());
			if (data.names == names && data.count == data.count)
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
