#pragma once

#include "bindings/Fields.h"

#include "definitions/enums/StorageMode.h"
#include "definitions/enums/PositioningMode.h"

namespace oly::editor
{
#define GLYPH_BODY_GENERATOR(M) \
		M((imtk::field::string_fld), texture_file) \
		M((imtk::field::int_fld<0, imp::nullpotential>), texture_slot) \
		M((imtk::field::rect_fld), location) \
		M((imtk::field::top_side_padding_fld), padding) \
		M((imtk::field::enum_fld<detail::PositioningMode>), origin_offset_mode) \
		M((imtk::field::vec2_fld<imp::nullpotential, imp::nullpotential>), origin_offset)

#define GLYPH_GENERATOR(M) \
		M((imtk::field::string_fld), codepoint) \
		GLYPH_BODY_GENERATOR(M)

	struct GlyphDesc
	{
		IMTK_DESCRIPTOR_BODY(GlyphDesc, GLYPH_GENERATOR);

		GlyphDesc(imtk::datapath_link link = {});
	};

#define RASTER_FONT_PARTIAL_GENERATOR(M) \
		M((imtk::field::float_fld<imp::nullpotential, imp::nullpotential>), space_advance_width) \
		M((imtk::field::float_fld<imp::nullpotential, imp::nullpotential>), line_height) \
		M((imtk::field::vec2_fld<0.f, imp::nullpotential>), font_scale) \
		M((imtk::field::enum_fld<detail::StorageMode>), storage)

#define RASTER_FONT_GENERATOR(M) \
		RASTER_FONT_PARTIAL_GENERATOR(M) \
		M((imtk::desc::vector<GlyphDesc>), glyphs)

	struct RasterFontDesc
	{
		IMTK_DESCRIPTOR_BODY(RasterFontDesc, RASTER_FONT_GENERATOR);

		RasterFontDesc(imtk::datapath_link link = {});
	};
}
