#include "FontFamilyDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	FontStyleDesc::FontStyleDesc(imtk::datapath_link link) :
		link(std::move(link)),
		font_file(IMTK_DATAPATH_SUBLINK(subpaths.font_file), "", detail::Key::File, "Font file"),
		atlas_index(IMTK_DATAPATH_SUBLINK(subpaths.atlas_index), 0, detail::Key::AtlasIndex, "Atlas index")
	{
	}

	FontFamilyDesc::FontFamilyDesc(imtk::datapath_link link) :
		link(std::move(link)),
		styles(detail::Key::Style, IMTK_DATAPATH_SUBLINK(subpaths.styles))
	{
	}
}
