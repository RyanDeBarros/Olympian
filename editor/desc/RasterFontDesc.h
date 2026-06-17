#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/StorageMode.h"
#include "definitions/enums/PositioningMode.h"

namespace oly::editor
{
	struct GlyphDesc
	{
		StringField codepoint;
		IntField<MakeOpt(0), MakeOpt<int>()> texture_index;
		IntField<MakeOpt(0), MakeOpt<int>()> texture_slot;
		RectField location;
		TopSidePaddingField padding;
		EnumField<detail::PositioningMode> origin_offset_mode;
		Vec2Field<MakeOpt<float>(), MakeOpt<float>()> origin_offset;

		GlyphDesc();
	};

	struct RasterFontDesc
	{
		FloatField<MakeOpt<float>(), MakeOpt<float>()> space_advance_width;
		FloatField<MakeOpt<float>(), MakeOpt<float>()> line_height;
		Vec2Field<MakeOpt(0.f), MakeOpt<float>()> font_scale;
		StringVectorField texture_files;
		VectorDesc<GlyphDesc> glyphs;
		static const detail::Key glyphs_key;
		EnumField<detail::StorageMode> storage;

		RasterFontDesc();
	};
}
