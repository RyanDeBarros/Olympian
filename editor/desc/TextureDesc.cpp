#include "TextureDesc.h"

#include "desc/DescIO.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	SpritesheetDesc::SpritesheetDesc() :
		col_type(detail::SpritesheetParamType::Index, detail::Key::ColType, "Column Type"),
		col_value(1, detail::Key::ColValue, ""),
		row_type(detail::SpritesheetParamType::Index, detail::Key::RowType, "Row Type"),
		row_value(1, detail::Key::RowValue, ""),
		col_offset_index(0, detail::Key::ColOffsetIndex, "Column offset (index)"),
		col_offset_pixel(0, detail::Key::ColOffsetPixel, "Column offset (pixels)"),
		row_offset_index(0, detail::Key::RowOffsetIndex, "Row offset (index)"),
		row_offset_pixel(0, detail::Key::RowOffsetPixel, "Row offset (pixels)"),
		delay(0.1f, detail::Key::Delay, "Delay (seconds)"),
		row_major(true, detail::Key::RowMajor, "Row Major"),
		row_up(true, detail::Key::RowUp, "Row Up")
	{
	}

	static const GLenum MIN_FILTER_VALUES[] = {
		GL_NEAREST,
		GL_LINEAR,
		GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_NEAREST,
		GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR
	};

	static const char* MIN_FILTER_NAMES[] = {
		"Nearest",
		"Linear",
		"Nearest (Nearest Mipmap)",
		"Linear (Nearest Mipmap)",
		"Nearest (Linear Mipmap)",
		"Linear (Linear Mipmap)"
	};

	static const GLenum MAG_FILTER_VALUES[] = {
		GL_NEAREST,
		GL_LINEAR,
	};

	static const char* MAG_FILTER_NAMES[] = {
		"Nearest",
		"Linear",
	};

	static const GLenum WRAP_VALUES[] = {
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_BORDER,
		GL_MIRRORED_REPEAT,
		GL_REPEAT,
		GL_MIRROR_CLAMP_TO_EDGE
	};

	static const char* WRAP_NAMES[] = {
		"Clamp To Edge",
		"Clamp To Border",
		"Repeat (Mirrored)",
		"Repeat",
		"Clamp To Edge (Mirrored)"
	};

	BaseTextureDesc::BaseTextureDesc(GLenum default_filter) :
		min_filter(default_filter, detail::Key::MinFilter, "Min Filter", MIN_FILTER_VALUES, MIN_FILTER_NAMES),
		mag_filter(default_filter, detail::Key::MagFilter, "Mag Filter", MAG_FILTER_VALUES, MAG_FILTER_NAMES),
		wrap_s(GL_CLAMP_TO_EDGE, detail::Key::WrapS, "Wrap (S)", WRAP_VALUES, WRAP_NAMES),
		wrap_t(GL_CLAMP_TO_EDGE, detail::Key::WrapT, "Wrap (T)", WRAP_VALUES, WRAP_NAMES),
		anim(false, detail::Key::Animated, "Animated"),
		spritesheet()
	{
	}

	RasterTextureDesc::RasterTextureDesc() :
		base(GL_NEAREST),
		generate_mipmaps(false, detail::Key::GenerateMipmaps, "Generate Mipmaps"),
		storage(detail::StorageMode::Discard, detail::Key::Storage, "Storage")
	{
	}

	VectorTextureDesc::VectorTextureDesc() :
		base(GL_LINEAR),
		generate_mipmaps(detail::SVGMipmapGenerationMode::Off, detail::Key::GenerateMipmaps, "Generate Mipmaps"),
		image_storage(detail::StorageMode::Discard, detail::Key::ImageStorage, "Image Storage"),
		abstract_storage(detail::StorageMode::Discard, detail::Key::AbstractStorage, "Abstract Storage"),
		scale(1.f, detail::Key::VectorScale, "Vector Scale")
	{
	}

	const detail::Key TextureVariantDesc::array_key = detail::Key::TextureArray;

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
