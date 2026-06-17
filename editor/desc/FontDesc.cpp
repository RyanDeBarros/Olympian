#include "FontDesc.h"

#include "definitions/Keys.h"
#include "definitions/enums/Filters.h"

namespace oly::editor
{
	const char* KERNING_SUBLABELS[2] = {
		"##1",
		"##2",
	};

	KerningDesc::KerningDesc() :
		pair({ "", "" }, detail::Key::CodepointPair, "Codepoints", KERNING_SUBLABELS),
		distance(0, detail::Key::CodepointDistance, "Distance")
	{
	}

	const detail::Key FontFaceDesc::kerning_key = detail::Key::Kerning;

	FontFaceDesc::FontFaceDesc() :
		storage(detail::StorageMode::Keep, detail::Key::Storage, "Storage")
	{
	}

	FontAtlasDesc::FontAtlasDesc() :
		font_size(36.f, detail::Key::FontSize, "Font size"),
		storage(detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		min_filter(GL_LINEAR, detail::Key::MinFilter, "Min filter", detail::MIN_FILTER_VALUES, detail::MIN_FILTER_NAMES),
		mag_filter(GL_LINEAR, detail::Key::MagFilter, "Mag filter", detail::MAG_FILTER_VALUES, detail::MAG_FILTER_NAMES),
		auto_generate_mipmaps(false, detail::Key::GenerateMipmaps, "Auto-generate mipmaps"),
		use_common_buffer_preset(true, detail::Key::UseCommonBufferPreset, "Use preset"),
		common_buffer_preset(detail::CommonBufferPreset::Common, detail::Key::CommonBufferPreset, "Preset"),
		common_buffer("", detail::Key::CommonBuffer, "Buffer")
	{
	}

	const detail::Key FullFontDesc::font_face_key = detail::Key::FontFace;
	const detail::Key FullFontDesc::font_atlas_key = detail::Key::FontAtlasArray;

	FullFontDesc::FullFontDesc() :
		font_face(),
		font_atlases()
	{
	}
}
