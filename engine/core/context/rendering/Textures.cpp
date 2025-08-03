#include "Textures.h"

#include "core/context/rendering/Sprites.h"
#include "registries/graphics/TextureRegistry.h"
#include "registries/graphics/primitives/Sprites.h"

namespace oly::context
{
	namespace internal
	{
		graphics::NSVGContext nsvg_context;
		reg::TextureRegistry texture_registry;
	}

	void internal::terminate_textures()
	{
		texture_registry.clear();
	}

	graphics::NSVGContext& nsvg_context()
	{
		return internal::nsvg_context;
	}

	reg::TextureRegistry& texture_registry()
	{
		return internal::texture_registry;
	}

	void sync_texture_handle(const graphics::BindlessTextureRef& texture)
	{
		sprite_batch().update_texture_handle(texture);
	}

	graphics::BindlessTextureRef load_texture(const std::string& file, unsigned int texture_index)
	{
		return internal::texture_registry.load_texture(file, { .texture_index = texture_index });
	}

	graphics::BindlessTextureRef load_svg_texture(const std::string& file, float svg_scale, unsigned int texture_index)
	{
		return internal::texture_registry.load_svg_texture(file, svg_scale, { .texture_index = texture_index });
	}

	glm::vec2 get_texture_dimensions(const std::string& file, unsigned int texture_index)
	{
		return internal::texture_registry.get_dimensions(file, texture_index);
	}
}
