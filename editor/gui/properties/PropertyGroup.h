#pragma once

#include "gui/properties/PropertyClipboard.h"

#include <memory>

namespace oly::editor
{
	struct PropertyGroup
	{
		static void Begin();
		static void End();

		static bool Append(std::unique_ptr<IPropertyView>&& prop);

		struct Indent
		{
			Indent();
			Indent(const Indent&) = delete;
			Indent(Indent&&) = delete;
			~Indent();
		};

		static bool CheckRow();
		static bool CheckHeader();
	};
}
