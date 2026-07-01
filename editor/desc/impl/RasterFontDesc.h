#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/StorageMode.h"
#include "definitions/enums/PositioningMode.h"

namespace oly::editor
{
#define GLYPH_BODY_GENERATOR(M) \
		M(texture_file) \
		M(texture_slot) \
		M(location) \
		M(padding) \
		M(origin_offset_mode) \
		M(origin_offset)

#define GLYPH_GENERATOR(M) \
		M(codepoint) \
		GLYPH_BODY_GENERATOR(M)

	struct GlyphDesc
	{
		StringField codepoint;
		StringField texture_file;
		IntField<MakeOpt(0), MakeOpt<int>()> texture_slot;
		RectField location;
		TopSidePaddingField padding;
		EnumField<detail::PositioningMode> origin_offset_mode;
		Vec2Field<MakeOpt<float>(), MakeOpt<float>()> origin_offset;

		DESCRIPTOR_BODY(GlyphDesc, GLYPH_GENERATOR);

		GlyphDesc();
	};

#define RASTER_FONT_PARTIAL_GENERATOR(M) \
		M(space_advance_width) \
		M(line_height) \
		M(font_scale) \
		M(storage)

#define RASTER_FONT_GENERATOR(M) \
		RASTER_FONT_PARTIAL_GENERATOR(M) \
		M(glyphs)

	struct RasterFontDesc
	{
		FloatField<MakeOpt<float>(), MakeOpt<float>()> space_advance_width;
		FloatField<MakeOpt<float>(), MakeOpt<float>()> line_height;
		Vec2Field<MakeOpt(0.f), MakeOpt<float>()> font_scale;
		EnumField<detail::StorageMode> storage;
		VectorDesc<GlyphDesc> glyphs;
		static const detail::Key glyphs_key;

		DESCRIPTOR_BODY(RasterFontDesc, RASTER_FONT_GENERATOR);

		RasterFontDesc();
	};
}
