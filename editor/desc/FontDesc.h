#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

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

	struct FontAtlasDesc
	{
		// TODO v8

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
