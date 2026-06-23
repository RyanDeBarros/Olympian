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

		class Indent
		{
			bool _active = false;

		public:
			Indent();
			Indent(const Indent&) = delete;
			Indent(Indent&&) noexcept;
			~Indent();
			Indent& operator=(Indent&&) = delete;
		};

		static bool CheckRow();
		static bool CheckHeader();
	};
}
