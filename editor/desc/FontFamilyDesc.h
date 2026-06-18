#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/FontStyle.h"

namespace oly::editor
{
#define STYLE_GENERATOR(M) \
		M(font_file) \
		M(atlas_index)

	struct FontStyleDesc
	{
		StringField font_file;
		IntField<MakeOpt(0), MakeOpt<int>()> atlas_index;

		FontStyleDesc();
	};

	struct FontFamilyDesc
	{
		MapDesc<detail::FontStyleMode, FontStyleDesc> styles;
		static const detail::Key styles_key;
	};
}
