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
		DESCRIPTOR_BODY(KerningDesc, KERNING_GENERATOR);

		StringArrayField<2> pair;
		IntField<MakeOpt<int>(), MakeOpt<int>()> distance;

		KerningDesc(DataPathLink link = {});

		friend std::ostream& operator<<(std::ostream& os, const KerningDesc& desc);
	};

#define FONT_FACE_PARTIAL_GENERATOR(M) \
		M(storage)

#define FONT_FACE_GENERATOR(M) \
		FONT_FACE_PARTIAL_GENERATOR(M) \
		M(kerning)

	struct FontFaceDesc
	{
		DESCRIPTOR_BODY(FontFaceDesc, FONT_FACE_GENERATOR);

		EnumField<detail::StorageMode> storage;
		VectorDesc<KerningDesc> kerning;
		static const detail::Key kerning_key;
		gui::DynamicListState kerning_ui_state;

		FontFaceDesc(DataPathLink link = {});
	};

#define FONT_ATLAS_NONPREVIEW_GENERATOR(M) \
		M(storage) \
		M(min_filter) \
		M(mag_filter) \
		M(auto_generate_mipmaps)

#define FONT_ATLAS_PARTIAL_GENERATOR(M) \
		M(font_size) \
		FONT_ATLAS_NONPREVIEW_GENERATOR(M)

#define FONT_ATLAS_GENERATOR(M) \
		FONT_ATLAS_PARTIAL_GENERATOR(M) \
		M(use_common_buffer_preset) \
		M(common_buffer_preset) \
		M(common_buffer)

	struct FontAtlasDesc
	{
		DESCRIPTOR_BODY(FontAtlasDesc, FONT_ATLAS_GENERATOR);

		FloatField<MakeOpt(1.f), MakeOpt<float>()> font_size;
		EnumField<detail::StorageMode> storage;
		DisjointEnumField<GLenum> min_filter;
		DisjointEnumField<GLenum> mag_filter;
		BoolField auto_generate_mipmaps;

		BoolField use_common_buffer_preset;
		EnumField<detail::CommonBufferPreset> common_buffer_preset;
		StringField common_buffer;

		FontAtlasDesc(DataPathLink link = {});
	};

#define FULL_FONT_GENERATOR(M) \
		M(font_face) \
		M(font_atlases)

	struct FullFontDesc
	{
		DESCRIPTOR_BODY(FullFontDesc, FULL_FONT_GENERATOR);

		FontFaceDesc font_face;
		static const detail::Key font_face_key;
		VectorDesc<FontAtlasDesc> font_atlases;
		static const detail::Key font_atlas_key;

		FullFontDesc(DataPathLink link = {});
	};
}
