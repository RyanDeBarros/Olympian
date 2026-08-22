#include "FontDesc.h"

#include "definitions/Keys.h"
#include "definitions/enums/Filters.h"

namespace oly::editor
{
	KerningDesc::KerningDesc(imtk::datapath_link link) :
		link(std::move(link)),
		pair(IMTK_DATAPATH_SUBLINK(subpaths.pair), { "", "" }, detail::Key::CodepointPair, "Codepoints"),
		distance(IMTK_DATAPATH_SUBLINK(subpaths.distance), 0, detail::Key::CodepointDistance, "Distance")
	{
	}

	std::ostream& operator<<(std::ostream& os, const KerningDesc& desc)
	{
		return os << "KerningDesc[pair=(\"" << desc.pair.value[0] << "\",\"" << desc.pair.value[1] << "\"), distance=" << desc.distance.value << "]";
	}

	FontFaceDesc::FontFaceDesc(imtk::datapath_link link) :
		link(std::move(link)),
		storage(IMTK_DATAPATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		kerning(detail::Key::Kerning, IMTK_DATAPATH_SUBLINK(subpaths.kerning))
	{
	}

	FontAtlasDesc::FontAtlasDesc(imtk::datapath_link link) :
		link(std::move(link)),
		font_size(IMTK_DATAPATH_SUBLINK(subpaths.font_size), 36.f, detail::Key::FontSize, "Font size"),
		storage(IMTK_DATAPATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage"),
		min_filter(IMTK_DATAPATH_SUBLINK(subpaths.min_filter), GL_LINEAR, detail::Key::MinFilter, "Min filter", detail::MIN_FILTER_VALUES, detail::MIN_FILTER_NAMES),
		mag_filter(IMTK_DATAPATH_SUBLINK(subpaths.mag_filter), GL_LINEAR, detail::Key::MagFilter, "Mag filter", detail::MAG_FILTER_VALUES, detail::MAG_FILTER_NAMES),
		auto_generate_mipmaps(IMTK_DATAPATH_SUBLINK(subpaths.auto_generate_mipmaps), false, detail::Key::GenerateMipmaps, "Auto-generate mipmaps"),
		use_common_buffer_preset(IMTK_DATAPATH_SUBLINK(subpaths.use_common_buffer_preset), true, detail::Key::UseCommonBufferPreset, "Use preset"),
		common_buffer_preset(IMTK_DATAPATH_SUBLINK(subpaths.common_buffer_preset), detail::CommonBufferPreset::Common, detail::Key::CommonBufferPreset, "Preset"),
		common_buffer(IMTK_DATAPATH_SUBLINK(subpaths.common_buffer), "", detail::Key::CommonBuffer, "Buffer")
	{
	}

	FullFontDesc::FullFontDesc(imtk::datapath_link link) :
		link(std::move(link)),
		font_face(detail::Key::FontFace, IMTK_DATAPATH_SUBLINK(subpaths.font_face)),
		font_atlases(detail::Key::FontAtlasArray, IMTK_DATAPATH_SUBLINK(subpaths.font_atlases))
	{
	}
}
