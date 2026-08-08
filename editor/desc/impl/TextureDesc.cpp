#include "TextureDesc.h"

#include "desc/DescIO.h"

#include "definitions/Keys.h"
#include "definitions/enums/Filters.h"

namespace oly::editor
{
	SpritesheetDesc::SpritesheetDesc(DataPathLink link) :
		link(std::move(link)),
		col_type(DATA_PATH_SUBLINK(subpaths.col_type), detail::SpritesheetParamType::Index, detail::Key::ColType, "Column Type"),
		col_value(DATA_PATH_SUBLINK(subpaths.col_value), 1, detail::Key::ColValue, ""),
		row_type(DATA_PATH_SUBLINK(subpaths.row_type), detail::SpritesheetParamType::Index, detail::Key::RowType, "Row Type"),
		row_value(DATA_PATH_SUBLINK(subpaths.row_value), 1, detail::Key::RowValue, ""),
		col_offset_index(DATA_PATH_SUBLINK(subpaths.col_offset_index), 0, detail::Key::ColOffsetIndex, "Column offset (index)"),
		col_offset_pixel(DATA_PATH_SUBLINK(subpaths.col_offset_pixel), 0, detail::Key::ColOffsetPixel, "Column offset (pixels)"),
		row_offset_index(DATA_PATH_SUBLINK(subpaths.row_offset_index), 0, detail::Key::RowOffsetIndex, "Row offset (index)"),
		row_offset_pixel(DATA_PATH_SUBLINK(subpaths.row_offset_pixel), 0, detail::Key::RowOffsetPixel, "Row offset (pixels)"),
		delay(DATA_PATH_SUBLINK(subpaths.delay), 0.1f, detail::Key::Delay, "Delay (seconds)"),
		row_major(DATA_PATH_SUBLINK(subpaths.row_major), true, detail::Key::RowMajor, "Row Major"),
		row_up(DATA_PATH_SUBLINK(subpaths.row_up), true, detail::Key::RowUp, "Row Up")
	{
	}

	BaseTextureDesc::BaseTextureDesc(GLenum default_filter, DataPathLink link) :
		link(std::move(link)),
		min_filter(DATA_PATH_SUBLINK(subpaths.min_filter), default_filter, detail::Key::MinFilter, "Min Filter", detail::MIN_FILTER_VALUES, detail::MIN_FILTER_NAMES),
		mag_filter(DATA_PATH_SUBLINK(subpaths.mag_filter), default_filter, detail::Key::MagFilter, "Mag Filter", detail::MAG_FILTER_VALUES, detail::MAG_FILTER_NAMES),
		wrap_s(DATA_PATH_SUBLINK(subpaths.wrap_s), GL_CLAMP_TO_EDGE, detail::Key::WrapS, "Wrap (S)", detail::WRAP_VALUES, detail::WRAP_NAMES),
		wrap_t(DATA_PATH_SUBLINK(subpaths.wrap_t), GL_CLAMP_TO_EDGE, detail::Key::WrapT, "Wrap (T)", detail::WRAP_VALUES, detail::WRAP_NAMES),
		anim(DATA_PATH_SUBLINK(subpaths.anim), false, detail::Key::Animated, "Animated"),
		spritesheet(DATA_PATH_SUBLINK(subpaths.spritesheet))
	{
	}

	RasterTextureDesc::RasterTextureDesc(DataPathLink link) :
		link(std::move(link)),
		base(GL_NEAREST, DATA_PATH_SUBLINK(subpaths.base)),
		generate_mipmaps(DATA_PATH_SUBLINK(subpaths.generate_mipmaps), false, detail::Key::GenerateMipmaps, "Generate Mipmaps"),
		storage(DATA_PATH_SUBLINK(subpaths.storage), detail::StorageMode::Keep, detail::Key::Storage, "Storage")
	{
	}

	VectorTextureDesc::VectorTextureDesc(DataPathLink link) :
		link(std::move(link)),
		base(GL_LINEAR, DATA_PATH_SUBLINK(subpaths.base)),
		generate_mipmaps(DATA_PATH_SUBLINK(subpaths.generate_mipmaps), detail::SVGMipmapGenerationMode::Off, detail::Key::GenerateMipmaps, "Generate Mipmaps"),
		image_storage(DATA_PATH_SUBLINK(subpaths.image_storage), detail::StorageMode::Keep, detail::Key::ImageStorage, "Image Storage"),
		abstract_storage(DATA_PATH_SUBLINK(subpaths.abstract_storage), detail::StorageMode::Discard, detail::Key::AbstractStorage, "Abstract Storage"),
		scale(DATA_PATH_SUBLINK(subpaths.scale), 1.f, detail::Key::VectorScale, "Vector Scale")
	{
	}

	const detail::Key TextureVariantDesc::array_key = detail::Key::TextureArray;

	TextureVariantDesc::TextureVariantDesc(DataPathLink link) :
		link(std::move(link)),
		variant(DATA_PATH_SUBLINK(subpaths.variant))
	{
	}

	size_t TextureVariantDesc::Size() const
	{
		return variant.Visit([](const auto& desc) { return desc.Size(); });
	}

	bool TextureVariantDesc::Empty() const
	{
		return Size() == 0;
	}

	void TextureVariantDesc::PushBack()
	{
		variant.Visit([](auto& desc) { desc.PushBack(); });
	}
	
	void TextureVariantDesc::Remove(size_t i)
	{
		variant.Visit([i](auto& desc) { desc.Remove(i); });
	}

	void TextureVariantDesc::Clear()
	{
		variant.Visit([](auto& desc) { desc.Clear(); });
	}
}
