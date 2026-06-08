#pragma once

#include "desc/Fields.h"
#include "desc/ArrayDesc.h"

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

		DESC_CHAIN_METHODS(SpritesheetDesc, SPRITESHEET_GENERATOR);
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

	struct BaseTextureDesc
	{
		GLenumField min_filter;
		GLenumField mag_filter;
		GLenumField wrap_s;
		GLenumField wrap_t;
		BoolField anim;
		SpritesheetDesc spritesheet;

		BaseTextureDesc(GLenum default_filter);

		DESC_CHAIN_METHODS(BaseTextureDesc, BASE_TEXTURE_GENERATOR);
	};

#define RASTER_TEXTURE_PARTIAL_GENERATOR(M) \
		M(generate_mipmaps) \
		M(storage)

#define RASTER_TEXTURE_FULL_GENERATOR(M) \
		M(base) \
		RASTER_TEXTURE_PARTIAL_GENERATOR(M)

	struct RasterTextureDesc
	{
		BaseTextureDesc base;
		BoolField generate_mipmaps;
		EnumField<detail::StorageMode> storage;

		RasterTextureDesc();

		DESC_CHAIN_METHODS(RasterTextureDesc, RASTER_TEXTURE_FULL_GENERATOR);
	};

#define VECTOR_TEXTURE_PARTIAL_GENERATOR(M) \
		M(generate_mipmaps) \
		M(image_storage) \
		M(abstract_storage) \
		M(scale)

#define VECTOR_TEXTURE_FULL_GENERATOR(M) \
		M(base) \
		VECTOR_TEXTURE_PARTIAL_GENERATOR(M)

	struct VectorTextureDesc
	{
		BaseTextureDesc base;
		EnumField<detail::SVGMipmapGenerationMode> generate_mipmaps;
		EnumField<detail::StorageMode> image_storage;
		EnumField<detail::StorageMode> abstract_storage;
		FloatField<MakeOpt(0.f), MakeOpt<float>()> scale;

		VectorTextureDesc();

		DESC_CHAIN_METHODS(VectorTextureDesc, VECTOR_TEXTURE_FULL_GENERATOR);
	};

	template<typename T>
	concept TextureSlotDesc = std::is_same_v<T, RasterTextureDesc> || std::is_same_v<T, VectorTextureDesc>;

	struct TextureVariantDesc
	{
		// TODO v8 use VariantDesc<ArrayDesc<RasterTextureDesc>, ArrayDesc<VectorTextureDesc>>
		std::variant<ArrayDesc<RasterTextureDesc>, ArrayDesc<VectorTextureDesc>> variant;
		static const detail::Key array_key;

		void Reset(TextureVariantDesc& source);
		void Isolate();
		size_t Count() const;
		bool Empty() const;
		void PushBack();
		void Remove(size_t i);
		IntField<MakeOpt(1), MakeOpt<int>()>& Size();
		void Resize(TextureVariantDesc& source);

		template<TextureSlotDesc SlotType>
		void Clear()
		{
			std::visit([](auto& desc) { desc.Clear(); }, variant);
			variant = ArrayDesc<SlotType>();
		}

		void Visit(auto&& visitor)
		{
			std::visit([&visitor](auto& desc) { desc.Visit(visitor); }, variant);
		}

		auto Visit(size_t i, auto&& visitor)
		{
			return std::visit([&visitor, i](auto& desc) {
				using T = std::invoke_result_t<decltype(visitor), decltype(*desc.array[i])>;
				if constexpr (std::is_same_v<T, void>)
				{
					return visitor(*desc.array[i]);
				}
				else
				{
					if (i < desc.array.size())
						return std::optional<T>(visitor(*desc.array[i]));
					else
						return std::optional<T>(std::nullopt);
				}
			}, variant);
		}

		void VisitIndexed(auto&& visitor)
		{
			std::visit([&visitor](auto& desc) { desc.VisitIndexed(visitor); }, variant);
		}
	};
}
