#pragma once

#include "desc/Fields.h"

#include <variant>
#include <vector>

namespace oly::editor
{
	struct SpritesheetDesc
	{
		IntField<MakeOpt(1), MakeOpt<int>()> rows;
		IntField<MakeOpt(1), MakeOpt<int>()> cols;
		IntField<MakeOpt(0), MakeOpt<int>()> cell_width_override;
		IntField<MakeOpt(0), MakeOpt<int>()> cell_height_override;
		IntField<MakeOpt(0), MakeOpt<int>()> delay_cs;
		BoolField row_major;
		BoolField row_up;

		SpritesheetDesc();

		void Reset(SpritesheetDesc& source);
	};

#define SPRITESHEET_GENERATOR(M) \
		M(rows) \
		M(cols) \
		M(cell_width_override) \
		M(cell_height_override) \
		M(delay_cs) \
		M(row_major) \
		M(row_up)

	struct BaseTextureDesc
	{
		GLenumField min_filter;
		GLenumField mag_filter;
		GLenumField wrap_s;
		GLenumField wrap_t;
		BoolField anim;
		SpritesheetDesc spritesheet;

		BaseTextureDesc();

		void Reset(BaseTextureDesc& source);
	};

#define TEXTURE_PARAMS_GENERATOR(M) \
		M(min_filter) \
		M(mag_filter) \
		M(wrap_s) \
		M(wrap_t)

#define BASE_TEXTURE_GENERATOR(M) \
		TEXTURE_PARAMS_GENERATOR(M) \
		M(anim) \
		M(spritesheet)

	struct RasterTextureDesc
	{
		BaseTextureDesc base;
		BoolField generate_mipmaps;
		EnumField<detail::StorageMode> storage;

		RasterTextureDesc();

		void Reset(RasterTextureDesc& source);
	};

#define RASTER_TEXTURE_PARTIAL_GENERATOR(M) \
		M(generate_mipmaps) \
		M(storage)

#define RASTER_TEXTURE_FULL_GENERATOR(M) \
		M(base) \
		RASTER_TEXTURE_PARTIAL_GENERATOR(M)

	struct VectorTextureDesc
	{
		BaseTextureDesc base;
		EnumField<detail::SVGMipmapGenerationMode> generate_mipmaps;
		EnumField<detail::StorageMode> image_storage;
		EnumField<detail::StorageMode> abstract_storage;
		FloatField<MakeOpt(0.f), MakeOpt<float>()> scale;

		VectorTextureDesc();

		void Reset(VectorTextureDesc& source);
	};

#define VECTOR_TEXTURE_PARTIAL_GENERATOR(M) \
		M(generate_mipmaps) \
		M(image_storage) \
		M(abstract_storage) \
		M(scale)

#define VECTOR_TEXTURE_FULL_GENERATOR(M) \
		M(base) \
		VECTOR_TEXTURE_PARTIAL_GENERATOR(M)

	struct TextureSlotDesc
	{
		std::variant<RasterTextureDesc, VectorTextureDesc> variant;

		void Reset(TextureSlotDesc& source);
	};

	struct TextureDesc
	{
		std::vector<TextureSlotDesc> array;

		void Reset(TextureDesc& source);
	};
}
