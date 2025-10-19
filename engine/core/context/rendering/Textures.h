#pragma once

#include "graphics/backend/basic/Textures.h"

namespace oly::graphics
{
	class NSVGContext;
}

namespace oly::reg
{
	class TextureRegistry;
}

namespace oly::context
{
	namespace internal
	{
		extern void terminate_textures();
	}

	extern graphics::NSVGContext& nsvg_context();

	extern reg::TextureRegistry& texture_registry();

	extern void sync_texture_handle(const graphics::BindlessTextureRef& texture);

	extern graphics::BindlessTextureRef load_texture(const ResourcePath& file, unsigned int texture_index = 0);
	extern graphics::BindlessTextureRef load_svg_texture(const ResourcePath& file, unsigned int texture_index = 0);
	extern glm::vec2 get_texture_dimensions(const ResourcePath& file, unsigned int texture_index = 0);
	extern glm::vec2 get_texture_dimensions(const graphics::BindlessTextureRef& texture);
	extern graphics::ImageDimensions get_image_dimensions(const ResourcePath& file, unsigned int texture_index = 0);
	extern graphics::ImageDimensions get_image_dimensions(const graphics::BindlessTextureRef& texture);
	extern std::weak_ptr<graphics::AnimDimensions> get_anim_dimensions(const ResourcePath& file, unsigned int texture_index = 0);
	extern std::weak_ptr<graphics::AnimDimensions> get_anim_dimensions(const graphics::BindlessTextureRef& texture);
}
