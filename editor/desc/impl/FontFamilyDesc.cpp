#include "FontFamilyDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	FontStyleDesc::FontStyleDesc(DataPathLink link) :
		link(std::move(link)),
		font_file(DATA_PATH_SUBLINK(subpaths.font_file), "", detail::Key::File, "Font file"),
		atlas_index(DATA_PATH_SUBLINK(subpaths.atlas_index), 0, detail::Key::AtlasIndex, "Atlas index")
	{
	}

	const detail::Key FontFamilyDesc::styles_key = detail::Key::Style;

	FontFamilyDesc::FontFamilyDesc(DataPathLink link) :
		link(std::move(link)),
		styles(DATA_PATH_SUBLINK(subpaths.styles))
	{
	}
}
