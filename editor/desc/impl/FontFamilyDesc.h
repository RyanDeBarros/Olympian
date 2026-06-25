#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/FontStyle.h"

namespace oly::editor
{
#define STYLE_GENERATOR(M) \
		M(font_file) \
		M(atlas_index)

#define STYLE_SUBPATH_GENERATOR(M) \
		STYLE_GENERATOR(M)

	struct FontStyleDesc
	{
		StringField font_file;
		IntField<MakeOpt(0), MakeOpt<int>()> atlas_index;

		GENERATE_SUBPATHS(STYLE_SUBPATH_GENERATOR);

		FontStyleDesc();
	};

#define FONT_FAMILY_SUBPATH_GENERATOR(M) \
		M(styles)

	struct FontFamilyDesc
	{
		MapDesc<detail::FontStyleMode, FontStyleDesc> styles;
		static const detail::Key styles_key;

		GENERATE_SUBPATHS(FONT_FAMILY_SUBPATH_GENERATOR);
	};
}
