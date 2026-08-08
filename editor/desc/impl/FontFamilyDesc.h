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
		DESCRIPTOR_BODY(FontStyleDesc, STYLE_GENERATOR);

		StringField font_file;
		IntField<MakeOpt(0), MakeOpt<int>()> atlas_index;

		FontStyleDesc(imtk::datapath_link link = {});
	};

#define FONT_FAMILY_GENERATOR(M) \
		M(styles)

	struct FontFamilyDesc
	{
		DESCRIPTOR_BODY(FontFamilyDesc, FONT_FAMILY_GENERATOR);

		MapDesc<detail::FontStyleMode, FontStyleDesc> styles;
		static const detail::Key styles_key;

		FontFamilyDesc(imtk::datapath_link link = {});
	};
}
