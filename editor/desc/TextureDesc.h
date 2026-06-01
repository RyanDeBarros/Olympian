#pragma once

#include "definitions/Enums.h"

#include "GL.h"

#include <optional>
#include <variant>
#include <vector>

namespace oly::editor
{
	struct SpritesheetDesc
	{
		int rows;
		int cols;
		int cell_width_override;
		int cell_height_override;
		int delay_cs;
		bool row_major;
		bool row_up;
	};

	struct BaseTextureDesc
	{
		GLenum min_filter;
		GLenum mag_filter;
		GLenum wrap_s;
		GLenum wrap_t;
		bool anim;
		SpritesheetDesc spritesheet;
	};

	struct RasterTextureDesc
	{
		BaseTextureDesc base;
		bool generate_mipmaps;
		detail::StorageMode storage;
	};

	struct VectorTextureDesc
	{
		BaseTextureDesc base;
		detail::SVGMipmapGenerationMode generate_mipmaps;
		detail::StorageMode image_storage;
		detail::StorageMode abstract_storage;
		float scale;
	};

	struct TextureSlotDesc
	{
		std::variant<RasterTextureDesc, VectorTextureDesc> variant;
	};

	struct TextureDesc
	{
		std::vector<TextureSlotDesc> array;
	};
}
