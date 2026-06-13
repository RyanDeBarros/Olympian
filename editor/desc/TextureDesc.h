#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/SpritesheetParamType.h"
#include "definitions/enums/StorageMode.h"
#include "definitions/enums/SVGMipmapGenerationMode.h"

#include <variant>
#include <vector>

namespace oly::editor
{
#define SPRITESHEET_PARTIAL_GENERATOR(M) \
		M(col_offset_index) \
		M(col_offset_pixel) \
		M(row_offset_index) \
		M(row_offset_pixel) \
		M(delay) \
		M(row_major) \
		M(row_up)

#define SPRITESHEET_GENERATOR(M) \
		M(col_type) \
		M(col_value) \
		M(row_type) \
		M(row_value) \
		SPRITESHEET_PARTIAL_GENERATOR(M)

	struct SpritesheetDesc
	{
		EnumField<detail::SpritesheetParamType> col_type;
		IntField<MakeOpt(1), MakeOpt<int>()> col_value;
		EnumField<detail::SpritesheetParamType> row_type;
		IntField<MakeOpt(1), MakeOpt<int>()> row_value;
		IntField<MakeOpt(0), MakeOpt<int>()> col_offset_index;
		IntField<MakeOpt(0), MakeOpt<int>()> col_offset_pixel;
		IntField<MakeOpt(0), MakeOpt<int>()> row_offset_index;
		IntField<MakeOpt(0), MakeOpt<int>()> row_offset_pixel;
		FloatField<MakeOpt(0.f), MakeOpt<float>()> delay;
		BoolField row_major;
		BoolField row_up;

		SpritesheetDesc();
	};

#define TEXTURE_PARAMS_GENERATOR(M) \
		M(min_filter) \
		M(mag_filter) \
		M(wrap_s) \
		M(wrap_t)

	struct BaseTextureDesc
	{
		DisjointEnumField<GLenum> min_filter;
		DisjointEnumField<GLenum> mag_filter;
		DisjointEnumField<GLenum> wrap_s;
		DisjointEnumField<GLenum> wrap_t;
		BoolField anim;
		SpritesheetDesc spritesheet;

		BaseTextureDesc(GLenum default_filter);
	};

#define RASTER_TEXTURE_PARTIAL_GENERATOR(M) \
		M(generate_mipmaps) \
		M(storage)

	struct RasterTextureDesc
	{
		BaseTextureDesc base;
		BoolField generate_mipmaps;
		EnumField<detail::StorageMode> storage;

		RasterTextureDesc();
	};

#define VECTOR_TEXTURE_PARTIAL_GENERATOR(M) \
		M(generate_mipmaps) \
		M(image_storage) \
		M(abstract_storage) \
		M(scale)

	struct VectorTextureDesc
	{
		BaseTextureDesc base;
		EnumField<detail::SVGMipmapGenerationMode> generate_mipmaps;
		EnumField<detail::StorageMode> image_storage;
		EnumField<detail::StorageMode> abstract_storage;
		FloatField<MakeOpt(0.f), MakeOpt<float>()> scale;

		VectorTextureDesc();
	};

	template<typename T>
	concept TextureSlotDesc = std::is_same_v<T, RasterTextureDesc> || std::is_same_v<T, VectorTextureDesc>;

	struct TextureVariantDesc
	{
		VariantDesc<VectorDesc<RasterTextureDesc>, VectorDesc<VectorTextureDesc>> variant;
		static const detail::Key array_key;

		size_t Size() const;
		bool Empty() const;
		void PushBack();
		void Remove(size_t i);
		void Clear();

		auto Visit(size_t i, auto&& visitor)
		{
			return variant.Visit([&visitor, i](auto& desc) {
				using T = std::invoke_result_t<decltype(visitor), decltype(desc[i])>;
				if constexpr (std::is_same_v<T, void>)
				{
					return visitor(desc[i]);
				}
				else
				{
					if (i < desc.Size())
						return std::optional<T>(visitor(desc[i]));
					else
						return std::optional<T>(std::nullopt);
				}
			});
		}

		void VisitIndexed(auto&& visitor)
		{
			variant.Visit([&visitor](auto& desc) {
				for (size_t i = 0; i < desc.Size(); ++i)
					visitor(i, desc[i]);
			});
		}
	};
}
