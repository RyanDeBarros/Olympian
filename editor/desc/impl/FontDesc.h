#pragma once

#include "desc/Fields.h"

#include "definitions/enums/CommonBufferPreset.h"
#include "definitions/enums/StorageMode.h"

namespace oly::editor
{
#define KERNING_GENERATOR(M) \
		M((StringArrayField<2>), pair) \
		M((IntField<imp::nullpotential, imp::nullpotential>), distance)

	struct KerningDesc
	{
		IMTK_DESCRIPTOR_BODY(KerningDesc, KERNING_GENERATOR);

		KerningDesc(imtk::datapath_link link = {});

		friend std::ostream& operator<<(std::ostream& os, const KerningDesc& desc);
	};

#define FONT_FACE_GENERATOR(M) \
		M((EnumField<detail::StorageMode>), storage) \
		M((imtk::desc::vector<KerningDesc>), kerning)

	struct FontFaceDesc
	{
		IMTK_DESCRIPTOR_BODY(FontFaceDesc, FONT_FACE_GENERATOR);

		gui::DynamicListState kerning_ui_state;

		FontFaceDesc(imtk::datapath_link link = {});
	};

#define FONT_ATLAS_NONPREVIEW_GENERATOR(M) \
		M((EnumField<detail::StorageMode>), storage) \
		M((DisjointEnumField<GLenum>), min_filter) \
		M((DisjointEnumField<GLenum>), mag_filter) \
		M((BoolField), auto_generate_mipmaps)

#define FONT_ATLAS_GENERATOR(M) \
		M((FloatField<1.f, imp::nullpotential>), font_size) \
		FONT_ATLAS_NONPREVIEW_GENERATOR(M) \
		M((BoolField), use_common_buffer_preset) \
		M((EnumField<detail::CommonBufferPreset>), common_buffer_preset) \
		M((StringField), common_buffer)

	struct FontAtlasDesc
	{
		IMTK_DESCRIPTOR_BODY(FontAtlasDesc, FONT_ATLAS_GENERATOR);

		FontAtlasDesc(imtk::datapath_link link = {});
	};

#define FULL_FONT_GENERATOR(M) \
		M((imtk::desc::sub<FontFaceDesc>), font_face) \
		M((imtk::desc::vector<FontAtlasDesc>), font_atlases)

	struct FullFontDesc
	{
		IMTK_DESCRIPTOR_BODY(FullFontDesc, FULL_FONT_GENERATOR);

		FullFontDesc(imtk::datapath_link link = {});
	};
}
