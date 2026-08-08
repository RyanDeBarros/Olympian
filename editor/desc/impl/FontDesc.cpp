#include "FontDesc.h"

#include "definitions/Keys.h"
#include "definitions/enums/Filters.h"

namespace oly::editor
{
	KerningDesc::KerningDesc(DataPathLink link) :
		link(std::move(link)),
		pair(DATA_PATH_SUBLINK(subpaths.pair), { "", "" }, detail::Key::CodepointPair, "Codepoints"),
		distance(DATA_PATH_SUBLINK(subpaths.distance), 0, detail::Key::CodepointDistance, "Distance")
	{
	}

	std::ostream& operator<<(std::ostream& os, const KerningDesc& desc)
	{
		return os << "KerningDesc[pair=(\"" << desc.pair.value[0] << "\",\"" << desc.pair.value[1] << "\"), distance=" << desc.distance.value << "]";
	}

	const detail::Key FontFaceDesc::kerning_key = detail::Key::Kerning;

	FontFaceDesc::FontFaceDesc(DataPathLink link) :
		link(std::move(link)),
		storage(DATA_PATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		kerning(DATA_PATH_SUBLINK(subpaths.kerning))
	{
	}

	FontAtlasDesc::FontAtlasDesc(DataPathLink link) :
		link(std::move(link)),
		font_size(DATA_PATH_SUBLINK(subpaths.font_size), 36.f, detail::Key::FontSize, "Font size"),
		storage(DATA_PATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		min_filter(DATA_PATH_SUBLINK(subpaths.min_filter), GL_LINEAR, detail::Key::MinFilter, "Min filter", detail::MIN_FILTER_VALUES, detail::MIN_FILTER_NAMES),
		mag_filter(DATA_PATH_SUBLINK(subpaths.mag_filter), GL_LINEAR, detail::Key::MagFilter, "Mag filter", detail::MAG_FILTER_VALUES, detail::MAG_FILTER_NAMES),
		auto_generate_mipmaps(DATA_PATH_SUBLINK(subpaths.auto_generate_mipmaps), false, detail::Key::GenerateMipmaps, "Auto-generate mipmaps"),
		use_common_buffer_preset(DATA_PATH_SUBLINK(subpaths.use_common_buffer_preset), true, detail::Key::UseCommonBufferPreset, "Use preset"),
		common_buffer_preset(DATA_PATH_SUBLINK(subpaths.common_buffer_preset), detail::CommonBufferPreset::Common, detail::Key::CommonBufferPreset, "Preset"),
		common_buffer(DATA_PATH_SUBLINK(subpaths.common_buffer), "", detail::Key::CommonBuffer, "Buffer")
	{
	}

	const detail::Key FullFontDesc::font_face_key = detail::Key::FontFace;
	const detail::Key FullFontDesc::font_atlas_key = detail::Key::FontAtlasArray;

	FullFontDesc::FullFontDesc(DataPathLink link) :
		link(std::move(link)),
		font_face(DATA_PATH_SUBLINK(subpaths.font_face)),
		font_atlases(DATA_PATH_SUBLINK(subpaths.font_atlases))
	{
	}
}
