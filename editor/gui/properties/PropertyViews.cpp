#include "PropertyViews.h"

namespace oly::editor::prop
{
	struct ComboPropertyPayload
	{
		int index;
		imtk::label_span_registry::handle names;
	};

	ComboPropertyView::ComboPropertyView(int& index, imtk::label_span_registry::handle names)
		: index(index), names(names)
	{
	}

	imtk::prop::payload ComboPropertyView::dump() const
	{
		return imtk::prop::payload::pod(ComboPropertyPayload{ .index = index, .names = names });
	}

	bool ComboPropertyView::can_load(const imtk::prop::payload& payload) const
	{
		if (auto data = payload.resolve<ComboPropertyPayload>())
			return data->names == names;
		else
			return false;
	}

	bool ComboPropertyView::try_load(const imtk::prop::payload& payload) const
	{
		if (auto data = payload.resolve<ComboPropertyPayload>())
		{
			if (data->names == names && index != data->index)
			{
				index = data->index;
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}
}
