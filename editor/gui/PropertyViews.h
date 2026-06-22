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
		const char** names;
		size_t count;

		ComboPropertyView(int& index, const char** names, size_t count);

		RawPropertyPayload Dump() const override;
		bool CanParse(const RawPropertyPayload& payload) const override;
		bool TryParse(const RawPropertyPayload& payload) const override;
	};
}
