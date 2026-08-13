#pragma once

#include "gui/properties/PropertyPayloads.h"

// TODO v9.3 remove after moving to imtk
#include <imtk.hpp>

namespace oly::editor::prop
{
	template<typename T>
	struct PrimitivePropertyView : public imtk::prop::iview
	{
		T& ref;

		PrimitivePropertyView(T& ref) : ref(ref) {}

		imtk::prop::payload dump() const override
		{
			return PropertyPayloadInterface<T>::Dump(ref);
		}

		bool can_load(const imtk::prop::payload& payload) const override
		{
			return imp::erase_type<T>() == payload.type;
		}
		
		bool try_load(const imtk::prop::payload& payload) const override
		{
			auto obj = PropertyPayloadInterface<T>::Load(payload);
			if (obj && ref != *obj)
			{
				ref = *obj;
				return true;
			}
			else
				return false;
		}
	};

	struct ComboPropertyView : public imtk::prop::iview
	{
		int& index;
		imtk::label_span_registry::handle names;
		
		ComboPropertyView(int& index, imtk::label_span_registry::handle names);

		imtk::prop::payload dump() const override;
		bool can_load(const imtk::prop::payload& payload) const override;
		bool try_load(const imtk::prop::payload& payload) const override;
	};
}
