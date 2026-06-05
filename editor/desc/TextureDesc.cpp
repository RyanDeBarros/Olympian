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
		delay_cs(0, detail::Key::DelayCS, "Delay (CS)"),
		row_major(true, detail::Key::RowMajor, "Row Major"),
		row_up(true, detail::Key::RowUp, "Row Up")
	{
	}

	void SpritesheetDesc::Reset(SpritesheetDesc& source)
	{
		RESET_FIELDS(SPRITESHEET_GENERATOR);
	}

	void SpritesheetDesc::Isolate()
	{
		ISOLATE_FIELDS(SPRITESHEET_GENERATOR);
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

	void BaseTextureDesc::Reset(BaseTextureDesc& source)
	{
		RESET_FIELDS(BASE_TEXTURE_GENERATOR);
	}

	void BaseTextureDesc::Isolate()
	{
		ISOLATE_FIELDS(BASE_TEXTURE_GENERATOR);
	}

	RasterTextureDesc::RasterTextureDesc() :
		base(GL_NEAREST),
		generate_mipmaps(false, detail::Key::GenerateMipmaps, "Generate Mipmaps"),
		storage(detail::StorageMode::Discard, detail::Key::Storage, "Storage")
	{
	}

	void RasterTextureDesc::Reset(RasterTextureDesc& source)
	{
		RESET_FIELDS(RASTER_TEXTURE_FULL_GENERATOR);
	}

	void RasterTextureDesc::Isolate()
	{
		ISOLATE_FIELDS(RASTER_TEXTURE_FULL_GENERATOR);
	}

	VectorTextureDesc::VectorTextureDesc() :
		base(GL_LINEAR),
		generate_mipmaps(detail::SVGMipmapGenerationMode::Off, detail::Key::GenerateMipmaps, "Generate Mipmaps"),
		image_storage(detail::StorageMode::Discard, detail::Key::ImageStorage, "Image Storage"),
		abstract_storage(detail::StorageMode::Discard, detail::Key::AbstractStorage, "Abstract Storage"),
		scale(1.f, detail::Key::VectorScale, "Vector Scale")
	{
	}

	void VectorTextureDesc::Reset(VectorTextureDesc& source)
	{
		RESET_FIELDS(VECTOR_TEXTURE_FULL_GENERATOR);
	}

	void VectorTextureDesc::Isolate()
	{
		ISOLATE_FIELDS(VECTOR_TEXTURE_FULL_GENERATOR);
	}

	void TextureDescVariant::Reset(TextureDescVariant& source)
	{
		Isolate();
		std::visit([this](auto& s) {
			using T = std::decay_t<decltype(s)>;
			variant = T();
			std::get<T>(variant).Reset(s);
		}, source.variant);
	}

	void TextureDescVariant::Isolate()
	{
		std::visit([](auto& desc) { desc.Isolate(); }, variant);
	}

	size_t TextureDescVariant::Count() const
	{
		return std::visit([](const auto& desc) { return desc.array.size(); }, variant);
	}

	bool TextureDescVariant::Empty() const
	{
		return Count() == 0;
	}

	void TextureDescVariant::PushBack()
	{
		std::visit([](auto& desc) { desc.PushBack(); }, variant);
	}
	
	void TextureDescVariant::Remove(size_t i)
	{
		std::visit([i](auto& desc) { desc.Remove(i); }, variant);
	}

	IntField<MakeOpt(1), MakeOpt<int>()>& TextureDescVariant::Size()
	{
		return std::visit([](auto& desc) -> IntField<MakeOpt(1), MakeOpt<int>()>&{ return desc.size; }, variant);
	}

	void TextureDescVariant::Resize(TextureDescVariant& source)
	{
		std::visit([this](auto& s) {
			using T = std::decay_t<decltype(s)>;
			variant = T();
			std::get<T>(variant).Resize(s);
		}, source.variant);
	}
}
