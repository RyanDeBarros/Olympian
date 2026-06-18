#include "RasterFontDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	GlyphDesc::GlyphDesc() :
		codepoint("", detail::Key::Codepoint, "Codepoint"),
		texture_file("", detail::Key::TextureFile, "Texture file"),
		texture_slot(0, detail::Key::TextureIndex, "Texture slot"),
		location(Rect::ZERO, detail::Key::Location, "Location"),
		padding({}, detail::Key::Padding, "Padding"),
		origin_offset_mode(detail::PositioningMode::Relative, detail::Key::OriginOffsetMode, "Origin offset mode"),
		origin_offset({}, detail::Key::OriginOffset, "Origin offset value")
	{
	}

	const detail::Key RasterFontDesc::glyphs_key = detail::Key::GlyphArray;

	RasterFontDesc::RasterFontDesc() :
		space_advance_width(5.f, detail::Key::SpaceAdvanceWidth, "Space advance width"),
		line_height(8.f, detail::Key::LineHeight, "Line height"),
		font_scale(glm::vec2(1.f), detail::Key::FontScale, "Font scale"),
		storage(detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		glyphs()
	{
	}
}
