#pragma once

#include "gui/properties/PropertyClipboard.h"

#include <memory>

namespace oly::editor
{
	struct PropertyGroup
	{
		static void Begin();
		static void End();

		static bool CheckValue(const IPropertyView& prop);
		static bool CheckRow(const PropertyRow& props);
		static bool CheckHeader(const PropertyPage& props);
	};
}
