#pragma once

#include "desc/Fields.h"

#include "definitions/enums/SpritesheetParamType.h"
#include "definitions/enums/StorageMode.h"
#include "definitions/enums/SVGMipmapGenerationMode.h"

#include <variant>
#include <vector>

namespace oly::editor
{
#define SPRITESHEET_PARTIAL_GENERATOR(M) \
		M((IntField<0, imp::nullpotential>), col_offset_index) \
		M((IntField<0, imp::nullpotential>), col_offset_pixel) \
		M((IntField<0, imp::nullpotential>), row_offset_index) \
		M((IntField<0, imp::nullpotential>), row_offset_pixel) \
		M((FloatField<0.f, imp::nullpotential>), delay) \
		M((BoolField), row_major) \
		M((BoolField), row_up)

#define SPRITESHEET_GENERATOR(M) \
		M((EnumField<detail::SpritesheetParamType>), col_type) \
		M((IntField<1, imp::nullpotential>), col_value) \
		M((EnumField<detail::SpritesheetParamType>), row_type) \
		M((IntField<1, imp::nullpotential>), row_value) \
		SPRITESHEET_PARTIAL_GENERATOR(M)

	struct SpritesheetDesc
	{
		IMTK_DESCRIPTOR_BODY(SpritesheetDesc, SPRITESHEET_GENERATOR);

		SpritesheetDesc(imtk::datapath_link link = {});
	};

#define TEXTURE_PARAMS_GENERATOR(M) \
		M((DisjointEnumField<GLenum>), min_filter) \
		M((DisjointEnumField<GLenum>), mag_filter) \
		M((DisjointEnumField<GLenum>), wrap_s) \
		M((DisjointEnumField<GLenum>), wrap_t)

#define BASE_TEXTURE_GENERATOR(M) \
		TEXTURE_PARAMS_GENERATOR(M) \
		M((BoolField), anim) \
		M((SpritesheetDesc), spritesheet)

	struct BaseTextureDesc
	{
		IMTK_DESCRIPTOR_BODY(BaseTextureDesc, BASE_TEXTURE_GENERATOR);

		BaseTextureDesc(GLenum default_filter, imtk::datapath_link link = {});
	};

#define RASTER_TEXTURE_PARTIAL_GENERATOR(M) \
		M((BoolField), generate_mipmaps) \
		M((EnumField<detail::StorageMode>), storage)

#define RASTER_TEXTURE_GENERATOR(M) \
		M((BaseTextureDesc), base) \
		RASTER_TEXTURE_PARTIAL_GENERATOR(M)

	struct RasterTextureDesc
	{
		IMTK_DESCRIPTOR_BODY(RasterTextureDesc, RASTER_TEXTURE_GENERATOR);

		RasterTextureDesc(imtk::datapath_link link = {});
	};

#define VECTOR_TEXTURE_PARTIAL_GENERATOR(M) \
		M((EnumField<detail::SVGMipmapGenerationMode>), generate_mipmaps) \
		M((EnumField<detail::StorageMode>), image_storage) \
		M((EnumField<detail::StorageMode>), abstract_storage) \
		M((FloatField<0.f, imp::nullpotential>), scale)

#define VECTOR_TEXTURE_GENERATOR(M) \
		M((BaseTextureDesc), base) \
		VECTOR_TEXTURE_PARTIAL_GENERATOR(M)

	struct VectorTextureDesc
	{
		IMTK_DESCRIPTOR_BODY(VectorTextureDesc, VECTOR_TEXTURE_GENERATOR);

		VectorTextureDesc(imtk::datapath_link link = {});
	};

#define TEXTURE_VARIANT_GENERATOR(M) \
		M((imtk::desc::subvariant<imtk::desc::vector<RasterTextureDesc>, imtk::desc::vector<VectorTextureDesc>>), variant)

	struct TextureVariantDesc
	{
		IMTK_DESCRIPTOR_BODY(TextureVariantDesc, TEXTURE_VARIANT_GENERATOR);

		TextureVariantDesc(imtk::datapath_link link = {});

		size_t Size() const;
		bool Empty() const;
		void PushBack();
		void Remove(size_t i);
		void Clear();

		auto Visit(size_t i, auto&& visitor)
		{
			return variant.visit([&visitor, i](auto& desc) {
				using T = std::invoke_result_t<decltype(visitor), decltype(desc[i])>;
				if constexpr (std::is_same_v<T, void>)
					return visitor(desc[i]);
				else
				{
					if (i < desc.size())
						return std::optional<T>(visitor(desc[i]));
					else
						return std::optional<T>(std::nullopt);
				}
			});
		}

		void VisitIndexed(auto&& visitor)
		{
			variant.visit([&visitor](auto& desc) {
				for (size_t i = 0; i < desc.size(); ++i)
					visitor(i, desc[i]);
			});
		}
	};
}
