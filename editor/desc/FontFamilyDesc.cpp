#include "FontFamilyDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	FontStyleDesc::FontStyleDesc() :
		font_file("", detail::Key::File, "Font file"),
		atlas_index(0, detail::Key::AtlasIndex, "Atlas index")
	{
	}

	const detail::Key FontFamilyDesc::styles_key = detail::Key::Style;
}
