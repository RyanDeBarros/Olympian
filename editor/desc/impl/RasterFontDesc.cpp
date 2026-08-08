#include "RasterFontDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	GlyphDesc::GlyphDesc(imtk::datapath_link link) :
		link(std::move(link)),
		codepoint(IMTK_DATAPATH_SUBLINK(subpaths.codepoint), "", detail::Key::Codepoint, "Codepoint"),
		texture_file(IMTK_DATAPATH_SUBLINK(subpaths.texture_file), "", detail::Key::TextureFile, "Texture file"),
		texture_slot(IMTK_DATAPATH_SUBLINK(subpaths.texture_slot), 0, detail::Key::TextureIndex, "Texture slot"),
		location(IMTK_DATAPATH_SUBLINK(subpaths.location), {}, detail::Key::Location, "Location"),
		padding(IMTK_DATAPATH_SUBLINK(subpaths.padding), {}, detail::Key::Padding, "Padding"),
		origin_offset_mode(IMTK_DATAPATH_SUBLINK(subpaths.origin_offset_mode), detail::PositioningMode::Relative, detail::Key::OriginOffsetMode, "Origin offset mode"),
		origin_offset(IMTK_DATAPATH_SUBLINK(subpaths.origin_offset), {}, detail::Key::OriginOffset, "Origin offset value")
	{
	}

	const detail::Key RasterFontDesc::glyphs_key = detail::Key::GlyphArray;

	RasterFontDesc::RasterFontDesc(imtk::datapath_link link) :
		link(std::move(link)),
		space_advance_width(IMTK_DATAPATH_SUBLINK(subpaths.space_advance_width), 5.f, detail::Key::SpaceAdvanceWidth, "Space advance width"),
		line_height(IMTK_DATAPATH_SUBLINK(subpaths.line_height), 8.f, detail::Key::LineHeight, "Line height"),
		font_scale(IMTK_DATAPATH_SUBLINK(subpaths.font_scale), glm::vec2(1.f), detail::Key::FontScale, "Font scale"),
		storage(IMTK_DATAPATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		glyphs(IMTK_DATAPATH_SUBLINK(subpaths.glyphs))
	{
	}
}
