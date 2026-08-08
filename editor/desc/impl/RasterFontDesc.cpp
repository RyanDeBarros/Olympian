#include "RasterFontDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	GlyphDesc::GlyphDesc(DataPathLink link) :
		link(std::move(link)),
		codepoint(DATA_PATH_SUBLINK(subpaths.codepoint), "", detail::Key::Codepoint, "Codepoint"),
		texture_file(DATA_PATH_SUBLINK(subpaths.texture_file), "", detail::Key::TextureFile, "Texture file"),
		texture_slot(DATA_PATH_SUBLINK(subpaths.texture_slot), 0, detail::Key::TextureIndex, "Texture slot"),
		location(DATA_PATH_SUBLINK(subpaths.location), {}, detail::Key::Location, "Location"),
		padding(DATA_PATH_SUBLINK(subpaths.padding), {}, detail::Key::Padding, "Padding"),
		origin_offset_mode(DATA_PATH_SUBLINK(subpaths.origin_offset_mode), detail::PositioningMode::Relative, detail::Key::OriginOffsetMode, "Origin offset mode"),
		origin_offset(DATA_PATH_SUBLINK(subpaths.origin_offset), {}, detail::Key::OriginOffset, "Origin offset value")
	{
	}

	const detail::Key RasterFontDesc::glyphs_key = detail::Key::GlyphArray;

	RasterFontDesc::RasterFontDesc(DataPathLink link) :
		link(std::move(link)),
		space_advance_width(DATA_PATH_SUBLINK(subpaths.space_advance_width), 5.f, detail::Key::SpaceAdvanceWidth, "Space advance width"),
		line_height(DATA_PATH_SUBLINK(subpaths.line_height), 8.f, detail::Key::LineHeight, "Line height"),
		font_scale(DATA_PATH_SUBLINK(subpaths.font_scale), glm::vec2(1.f), detail::Key::FontScale, "Font scale"),
		storage(DATA_PATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		glyphs(DATA_PATH_SUBLINK(subpaths.glyphs))
	{
	}
}
