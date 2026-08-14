#pragma once

#include "desc/Fields.h"

#include "definitions/enums/FontStyle.h"

namespace oly::editor
{
#define STYLE_GENERATOR(M) \
		M((StringField), font_file) \
		M((IntField<MakeOpt(0), MakeOpt<int>()>), atlas_index)

	struct FontStyleDesc
	{
		IMTK_DESCRIPTOR_BODY(FontStyleDesc, STYLE_GENERATOR);

		FontStyleDesc(imtk::datapath_link link = {});
	};

#define FONT_FAMILY_GENERATOR(M) \
		M((imtk::desc::map<detail::FontStyleMode, FontStyleDesc>), styles)

	struct FontFamilyDesc
	{
		IMTK_DESCRIPTOR_BODY(FontFamilyDesc, FONT_FAMILY_GENERATOR);

		static const detail::Key styles_key;

		FontFamilyDesc(imtk::datapath_link link = {});
	};
}
