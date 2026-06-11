#include "FontDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* KERNING_SUBLABELS[2] = {
		"##1",
		"##2",
	};

	KerningDesc::KerningDesc() :
		pair({ "", "" }, detail::Key::CodepointPair, "Codepoints", KERNING_SUBLABELS),
		distance(0, detail::Key::CodepointDistance, "Distance")
	{
	}

	const detail::Key FontFaceDesc::kerning_key = detail::Key::Kerning;

	FontFaceDesc::FontFaceDesc() :
		storage(detail::StorageMode::Discard, detail::Key::Storage, "Storage")
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
