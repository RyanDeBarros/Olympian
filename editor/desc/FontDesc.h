#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/CommonBufferPreset.h"
#include "definitions/enums/StorageMode.h"

namespace oly::editor
{
#define KERNING_GENERATOR(M) \
		M(pair) \
		M(distance)

	struct KerningDesc
	{
		ArrayField<std::string, 2> pair;
		IntField<MakeOpt<int>(), MakeOpt<int>()> distance;

		KerningDesc();
	};

#define FONT_FACE_PARTIAL_GENERATOR(M) \
		M(storage)

	struct FontFaceDesc
	{
		EnumField<detail::StorageMode> storage;
		VectorDesc<KerningDesc> kerning;
		static const detail::Key kerning_key;
		gui::DynamicListState kerning_ui_state;

		FontFaceDesc();
	};

#define FONT_ATLAS_PARTIAL_GENERATOR(M) \
		M(font_size) \
		M(storage) \
		M(min_filter) \
		M(mag_filter) \
		M(auto_generate_mipmaps)

#define FONT_ATLAS_GENERATOR(M) \
		FONT_ATLAS_PARTIAL_GENERATOR(M) \
		M(use_common_buffer_preset) \
		M(common_buffer_preset) \
		M(common_buffer)

	struct FontAtlasDesc
	{
		FloatField<MakeOpt(1.f), MakeOpt<float>()> font_size;
		EnumField<detail::StorageMode> storage;
		DisjointEnumField<GLenum> min_filter;
		DisjointEnumField<GLenum> mag_filter;
		BoolField auto_generate_mipmaps;

		BoolField use_common_buffer_preset;
		EnumField<detail::CommonBufferPreset> common_buffer_preset;
		StringField common_buffer;

		FontAtlasDesc();
	};

	struct FullFontDesc
	{
		FontFaceDesc font_face;
		static const detail::Key font_face_key;
		VectorDesc<FontAtlasDesc> font_atlases;
		static const detail::Key font_atlas_key;

		FullFontDesc();
	};
}
