#include "TextureDesc.h"

#include "desc/DescIO.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	SpritesheetDesc::SpritesheetDesc() :
		rows(1, detail::Key::Rows, "Rows"),
		cols(1, detail::Key::Columns, "Columns"),
		cell_width_override(0, detail::Key::CellWidthOverride, "Cell Width Override"),
		cell_height_override(0, detail::Key::CellHeightOverride, "Cell Height Override"),
		delay_cs(0, detail::Key::DelayCS, "Delay (CS)"),
		row_major(true, detail::Key::RowMajor, "Row Major"),
		row_up(true, detail::Key::RowUp, "Row Up")
	{
	}

	void SpritesheetDesc::Reset(SpritesheetDesc& source)
	{
		RESET_FIELDS(SPRITESHEET_GENERATOR);
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

	BaseTextureDesc::BaseTextureDesc() :
		min_filter(GL_NEAREST, detail::Key::MinFilter, "Min Filter", MIN_FILTER_VALUES, MIN_FILTER_NAMES),
		mag_filter(GL_NEAREST, detail::Key::MagFilter, "Mag Filter", MAG_FILTER_VALUES, MAG_FILTER_NAMES),
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

	RasterTextureDesc::RasterTextureDesc() :
		base(),
		generate_mipmaps(false, detail::Key::GenerateMipmaps, "Generate Mipmaps"),
		storage(detail::StorageMode::Discard, detail::Key::Storage, "Storage")
	{
	}

	void RasterTextureDesc::Reset(RasterTextureDesc& source)
	{
		RESET_FIELDS(RASTER_TEXTURE_FULL_GENERATOR);
	}

	VectorTextureDesc::VectorTextureDesc() :
		base(),
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

	void TextureSlotDesc::Reset(TextureSlotDesc& source)
	{
		std::visit([this](auto& s) {
			using T = std::decay_t<decltype(s)>;
			variant = T();
			std::get<T>(variant).Reset(s);
		}, source.variant);
	}

	void TextureDesc::Reset(TextureDesc& source)
	{
		array.clear();
		array.resize(source.array.size());
		for (size_t i = 0; i < array.size(); ++i)
			array[i].Reset(source.array[i]);
	}
}
