#pragma once

#include "gui/PropertyClipboard.h"
#include "gui/PropertyPayloads.h"

namespace oly::editor::prop
{
	template<typename T>
	struct PrimitivePropertyView : public IPropertyView
	{
		T& ref;

		PrimitivePropertyView(T& ref) : ref(ref) {}

		virtual RawPropertyPayload Dump() const
		{
			return MakePropertyPayload(ref);
		}

		virtual bool CanParse(const RawPropertyPayload& payload) const
		{
			return ParsePropertyPayload<T>(payload).has_value();
		}
		
		virtual bool TryParse(const RawPropertyPayload& payload)
		{
			if (auto value = ParsePropertyPayload<T>(payload))
			{
				if (ref != *value)
				{
					ref = *value;
					return true;
				}
				else
					return false;
			}
			else
				return false;
		}
	};
}
