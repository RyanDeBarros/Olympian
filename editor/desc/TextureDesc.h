#pragma once

#include "desc/Fields.h"
#include "definitions/enums/StorageMode.h"
#include "definitions/enums/SVGMipmapGenerationMode.h"

#include <variant>
#include <vector>

namespace oly::editor
{
	struct SpritesheetDesc
	{
		IntField<MakeOpt(1), MakeOpt<int>()> rows;
		IntField<MakeOpt(1), MakeOpt<int>()> cols;
		BoolField enable_cell_width_override;
		BoolField enable_cell_height_override;
		IntField<MakeOpt(1), MakeOpt<int>()> cell_width_override;
		IntField<MakeOpt(1), MakeOpt<int>()> cell_height_override;
		IntField<MakeOpt(0), MakeOpt<int>()> delay_cs;
		BoolField row_major;
		BoolField row_up;

		SpritesheetDesc();

		void Reset(SpritesheetDesc& source);
		void Isolate();
	};

#define SPRITESHEET_GENERATOR(M) \
		M(rows) \
		M(cols) \
		M(enable_cell_width_override) \
		M(cell_width_override) \
		M(enable_cell_height_override) \
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
		void Isolate();
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
		void Isolate();
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
		void Isolate();
	};

#define VECTOR_TEXTURE_PARTIAL_GENERATOR(M) \
		M(generate_mipmaps) \
		M(image_storage) \
		M(abstract_storage) \
		M(scale)

#define VECTOR_TEXTURE_FULL_GENERATOR(M) \
		M(base) \
		VECTOR_TEXTURE_PARTIAL_GENERATOR(M)

	template<typename T>
	concept TextureSlotDesc = std::is_same_v<T, RasterTextureDesc> || std::is_same_v<T, VectorTextureDesc>;

	template<TextureSlotDesc SlotType>
	struct TextureDesc
	{
		std::vector<std::unique_ptr<SlotType>> array;

		void Reset(TextureDesc& source)
		{
			Clear();
			array.resize(source.array.size());
			for (size_t i = 0; i < array.size(); ++i)
			{
				array[i] = std::make_unique<SlotType>();
				array[i]->Reset(*source.array[i]);
			}
		}

		void Isolate()
		{
			for (auto& desc : array)
				desc->Isolate();
		}

		void PushBack()
		{
			array.push_back(std::make_unique<SlotType>());
		}

		void Remove(size_t i)
		{
			array[i]->Isolate();
			array.erase(array.begin() + i);
		}

		void Clear()
		{
			Isolate();
			array.clear();
		}
	};

	struct TextureDescVariant
	{
		std::variant<TextureDesc<RasterTextureDesc>, TextureDesc<VectorTextureDesc>> variant;

		void Reset(TextureDescVariant& source);
		void Isolate();
		size_t Count() const;
		bool Empty() const;
		void PushBack();
		void Remove(size_t i);

		template<TextureSlotDesc SlotType>
		void Clear()
		{
			std::visit([](auto& desc) { desc.Clear(); }, variant);
			variant = TextureDesc<SlotType>();
		}

		void Visit(auto&& visitor)
		{
			std::visit([&visitor](auto& desc) {
				for (auto& d : desc.array)
					visitor(*d);
			}, variant);
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
			std::visit([&visitor](auto& desc) {
				for (size_t i = 0; i < desc.array.size(); ++i)
					visitor(i, *desc.array[i]);
			}, variant);
		}
	};
}
