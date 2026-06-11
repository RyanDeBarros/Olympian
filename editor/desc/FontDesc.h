#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

namespace oly::editor
{
	struct FontFaceDesc
	{
		// TODO v8 storage
		// TODO v8 kerning

		FontFaceDesc();
	};

	struct FontAtlasDesc
	{
		// TODO v8

		FontAtlasDesc();
	};

	struct FullFontDesc
	{
		FontFaceDesc font_face;
		static const detail::Key font_face_key;
		VectorDesc<FontAtlasDesc> font_atlases;
		static const detail::Key font_atlas_key;

		FullFontDesc();
	};
}
