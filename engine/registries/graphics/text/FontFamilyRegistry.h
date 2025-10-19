#pragma once

#include "graphics/text/FontFamily.h"
#include "core/util/ResourcePath.h"

namespace oly::reg
{
	class FontFamilyRegistry
	{
		std::unordered_map<ResourcePath, rendering::FontFamilyRef> font_families;

	public:
		void clear();

		rendering::FontFamilyRef load_font_family(const ResourcePath& file);
		void free_font_family(const ResourcePath& file);
	};
}
