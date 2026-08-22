#include "TextureDesc.h"

#include "desc/DescIO.h"

#include "definitions/Keys.h"
#include "definitions/enums/Filters.h"

namespace oly::editor
{
	SpritesheetDesc::SpritesheetDesc(imtk::datapath_link link) :
		link(std::move(link)),
		col_type(IMTK_DATAPATH_SUBLINK(subpaths.col_type), detail::SpritesheetParamType::Index, detail::Key::ColType, "Column Type"),
		col_value(IMTK_DATAPATH_SUBLINK(subpaths.col_value), 1, detail::Key::ColValue, ""),
		row_type(IMTK_DATAPATH_SUBLINK(subpaths.row_type), detail::SpritesheetParamType::Index, detail::Key::RowType, "Row Type"),
		row_value(IMTK_DATAPATH_SUBLINK(subpaths.row_value), 1, detail::Key::RowValue, ""),
		col_offset_index(IMTK_DATAPATH_SUBLINK(subpaths.col_offset_index), 0, detail::Key::ColOffsetIndex, "Column offset (index)"),
		col_offset_pixel(IMTK_DATAPATH_SUBLINK(subpaths.col_offset_pixel), 0, detail::Key::ColOffsetPixel, "Column offset (pixels)"),
		row_offset_index(IMTK_DATAPATH_SUBLINK(subpaths.row_offset_index), 0, detail::Key::RowOffsetIndex, "Row offset (index)"),
		row_offset_pixel(IMTK_DATAPATH_SUBLINK(subpaths.row_offset_pixel), 0, detail::Key::RowOffsetPixel, "Row offset (pixels)"),
		delay(IMTK_DATAPATH_SUBLINK(subpaths.delay), 0.1f, detail::Key::Delay, "Delay (seconds)"),
		row_major(IMTK_DATAPATH_SUBLINK(subpaths.row_major), true, detail::Key::RowMajor, "Row Major"),
		row_up(IMTK_DATAPATH_SUBLINK(subpaths.row_up), true, detail::Key::RowUp, "Row Up")
	{
	}

	BaseTextureDesc::BaseTextureDesc(GLenum default_filter, imtk::datapath_link link) :
		link(std::move(link)),
		min_filter(IMTK_DATAPATH_SUBLINK(subpaths.min_filter), default_filter, detail::Key::MinFilter, "Min Filter", detail::MIN_FILTER_VALUES, detail::MIN_FILTER_NAMES),
		mag_filter(IMTK_DATAPATH_SUBLINK(subpaths.mag_filter), default_filter, detail::Key::MagFilter, "Mag Filter", detail::MAG_FILTER_VALUES, detail::MAG_FILTER_NAMES),
		wrap_s(IMTK_DATAPATH_SUBLINK(subpaths.wrap_s), GL_CLAMP_TO_EDGE, detail::Key::WrapS, "Wrap (S)", detail::WRAP_VALUES, detail::WRAP_NAMES),
		wrap_t(IMTK_DATAPATH_SUBLINK(subpaths.wrap_t), GL_CLAMP_TO_EDGE, detail::Key::WrapT, "Wrap (T)", detail::WRAP_VALUES, detail::WRAP_NAMES),
		anim(IMTK_DATAPATH_SUBLINK(subpaths.anim), false, detail::Key::Animated, "Animated"),
		spritesheet(IMTK_DATAPATH_SUBLINK(subpaths.spritesheet))
	{
	}

	RasterTextureDesc::RasterTextureDesc(imtk::datapath_link link) :
		link(std::move(link)),
		base(GL_NEAREST, IMTK_DATAPATH_SUBLINK(subpaths.base)),
		generate_mipmaps(IMTK_DATAPATH_SUBLINK(subpaths.generate_mipmaps), false, detail::Key::GenerateMipmaps, "Generate Mipmaps"),
		storage(IMTK_DATAPATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage")
	{
	}

	VectorTextureDesc::VectorTextureDesc(imtk::datapath_link link) :
		link(std::move(link)),
		base(GL_LINEAR, IMTK_DATAPATH_SUBLINK(subpaths.base)),
		generate_mipmaps(IMTK_DATAPATH_SUBLINK(subpaths.generate_mipmaps), detail::SVGMipmapGenerationMode::Off, detail::Key::GenerateMipmaps, "Generate Mipmaps"),
		image_storage(IMTK_DATAPATH_SUBLINK(subpaths.image_storage), detail::StorageMode::Keep, detail::Key::ImageStorage, "Image Storage"),
		abstract_storage(IMTK_DATAPATH_SUBLINK(subpaths.abstract_storage), detail::StorageMode::Discard, detail::Key::AbstractStorage, "Abstract Storage"),
		scale(IMTK_DATAPATH_SUBLINK(subpaths.scale), 1.f, detail::Key::VectorScale, "Vector Scale")
	{
	}

	TextureVariantDesc::TextureVariantDesc(imtk::datapath_link link) :
		link(std::move(link)),
		variant(detail::Key::TextureArray, IMTK_DATAPATH_SUBLINK(subpaths.variant))
	{
	}

	size_t TextureVariantDesc::Size() const
	{
		return variant.visit([](const auto& desc) { return desc.size(); });
	}

	bool TextureVariantDesc::Empty() const
	{
		return Size() == 0;
	}

	void TextureVariantDesc::PushBack()
	{
		variant.visit([](auto& desc) { desc.push_back(); });
	}
	
	void TextureVariantDesc::Remove(size_t i)
	{
		variant.visit([i](auto& desc) { desc.remove(i); });
	}

	void TextureVariantDesc::Clear()
	{
		variant.visit([](auto& desc) { desc.clear(); });
	}
}
