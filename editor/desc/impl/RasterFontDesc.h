#pragma once

#include "desc/Fields.h"

#include "definitions/enums/StorageMode.h"
#include "definitions/enums/PositioningMode.h"

namespace oly::editor
{
#define GLYPH_BODY_GENERATOR(M) \
		M((StringField), texture_file) \
		M((IntField<MakeOpt(0), MakeOpt<int>()>), texture_slot) \
		M((RectField), location) \
		M((TopSidePaddingField), padding) \
		M((EnumField<detail::PositioningMode>), origin_offset_mode) \
		M((Vec2Field<MakeOpt<float>(), MakeOpt<float>()>), origin_offset)

#define GLYPH_GENERATOR(M) \
		M((StringField), codepoint) \
		GLYPH_BODY_GENERATOR(M)

	struct GlyphDesc
	{
		IMTK_DESCRIPTOR_BODY(GlyphDesc, GLYPH_GENERATOR);

		GlyphDesc(imtk::datapath_link link = {});
	};

#define RASTER_FONT_PARTIAL_GENERATOR(M) \
		M((FloatField<MakeOpt<float>(), MakeOpt<float>()>), space_advance_width) \
		M((FloatField<MakeOpt<float>(), MakeOpt<float>()>), line_height) \
		M((Vec2Field<MakeOpt(0.f), MakeOpt<float>()>), font_scale) \
		M((EnumField<detail::StorageMode>), storage)

#define RASTER_FONT_GENERATOR(M) \
		RASTER_FONT_PARTIAL_GENERATOR(M) \
		M((imtk::desc::vector<GlyphDesc>), glyphs)

	struct RasterFontDesc
	{
		IMTK_DESCRIPTOR_BODY(RasterFontDesc, RASTER_FONT_GENERATOR);

		static const detail::Key glyphs_key;

		RasterFontDesc(imtk::datapath_link link = {});
	};
}
