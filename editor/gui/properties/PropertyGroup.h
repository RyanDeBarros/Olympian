#pragma once

#include "gui/properties/PropertyClipboard.h"

#include <memory>

namespace oly::editor
{
	struct PropertyGroup
	{
		static void Clear();
		static bool Append(std::unique_ptr<IPropertyView>&& prop);
		static bool Submit();
	};
}
