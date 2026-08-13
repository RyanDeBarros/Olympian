#pragma once

#include "gui/properties/PropertyClipboard.h"
#include "gui/properties/PropertyPayloads.h"

// TODO v9.3 remove after moving to imtk
#include <imtk.hpp>

namespace oly::editor::prop
{
	template<typename T>
	struct PrimitivePropertyView : public IPropertyView
	{
		T& ref;

		PrimitivePropertyView(T& ref) : ref(ref) {}

		RawPropertyPayload Dump() const override
		{
			return MakePropertyPayload(ref);
		}

		bool CanParse(const RawPropertyPayload& payload) const override
		{
			return CanParsePropertyPayload<T>(payload);
		}
		
		bool TryParse(const RawPropertyPayload& payload) const override
		{
			const T og = ref;
			if (TryParsePropertyPayload(payload, ref))
				return ref != og;
			else
				return false;
		}
	};

	struct ComboPropertyView : public IPropertyView
	{
		int& index;
		imtk::label_span_registry::handle names;
		
		ComboPropertyView(int& index, imtk::label_span_registry::handle names);

		RawPropertyPayload Dump() const override;
		bool CanParse(const RawPropertyPayload& payload) const override;
		bool TryParse(const RawPropertyPayload& payload) const override;
	};
}
