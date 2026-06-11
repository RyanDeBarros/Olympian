#include "FontDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	FontFaceDesc::FontFaceDesc()
	{
	}

	FontAtlasDesc::FontAtlasDesc()
	{
	}

	const detail::Key FullFontDesc::font_face_key = detail::Key::FontFace;
	const detail::Key FullFontDesc::font_atlas_key = detail::Key::FontAtlasArray;

	FullFontDesc::FullFontDesc() :
		font_face(),
		font_atlases()
	{
	}
}
