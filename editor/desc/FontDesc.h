#pragma once

#include "bindings/Fields.h"

#include "definitions/enums/CommonBufferPreset.h"
#include "definitions/enums/StorageMode.h"

namespace oly::editor
{
#define KERNING_GENERATOR(M) \
		M((imtk::field::array_fld<imtk::field::string_fld, 2>), pair) \
		M((imtk::field::int_fld<imp::nullpotential, imp::nullpotential>), distance)

	struct KerningDesc
	{
		IMTK_DESCRIPTOR_BODY(KerningDesc, KERNING_GENERATOR);

		KerningDesc(imtk::datapath_link link = {});

		friend std::ostream& operator<<(std::ostream& os, const KerningDesc& desc);
	};

#define FONT_FACE_GENERATOR(M) \
		M((imtk::field::enum_fld<detail::StorageMode>), storage) \
		M((imtk::desc::vector<KerningDesc>), kerning)

	struct FontFaceDesc
	{
		IMTK_DESCRIPTOR_BODY(FontFaceDesc, FONT_FACE_GENERATOR);

		imtk::dynamic_list kerning_widget;

		FontFaceDesc(imtk::datapath_link link = {});
	};

#define FONT_ATLAS_NONPREVIEW_GENERATOR(M) \
		M((imtk::field::enum_fld<detail::StorageMode>), storage) \
		M((imtk::field::disjoint_enum_fld<GLenum>), min_filter) \
		M((imtk::field::disjoint_enum_fld<GLenum>), mag_filter) \
		M((imtk::field::bool_fld), auto_generate_mipmaps)

#define FONT_ATLAS_GENERATOR(M) \
		M((imtk::field::float_fld<1.f, imp::nullpotential>), font_size) \
		FONT_ATLAS_NONPREVIEW_GENERATOR(M) \
		M((imtk::field::bool_fld), use_common_buffer_preset) \
		M((imtk::field::enum_fld<detail::CommonBufferPreset>), common_buffer_preset) \
		M((imtk::field::string_fld), common_buffer)

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
