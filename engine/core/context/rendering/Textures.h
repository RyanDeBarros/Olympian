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

	extern graphics::BindlessTextureRef load_texture(const std::string& file, unsigned int texture_index = 0);
	extern graphics::BindlessTextureRef load_svg_texture(const std::string& file, unsigned int texture_index = 0);
	extern glm::vec2 get_texture_dimensions(const std::string& file, unsigned int texture_index = 0);
}
