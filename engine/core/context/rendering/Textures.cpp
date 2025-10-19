#include "Textures.h"

#include "core/context/rendering/Sprites.h"
#include "registries/graphics/TextureRegistry.h"
#include "registries/graphics/sprites/Sprites.h"
#include "Textures.h"

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
		rendering::internal::SpriteBatchRegistry::instance().update_texture_handle(texture);
	}

	graphics::BindlessTextureRef load_texture(const ResourcePath& file, unsigned int texture_index)
	{
		return internal::texture_registry.load_texture(file, texture_index);
	}

	graphics::BindlessTextureRef load_svg_texture(const ResourcePath& file, unsigned int texture_index)
	{
		return internal::texture_registry.load_svg_texture(file, texture_index);
	}

	glm::vec2 get_texture_dimensions(const ResourcePath& file, unsigned int texture_index)
	{
		return internal::texture_registry.get_dimensions(file, texture_index);
	}

	glm::vec2 get_texture_dimensions(const graphics::BindlessTextureRef& texture)
	{
		return internal::texture_registry.get_dimensions(texture);
	}

	graphics::ImageDimensions get_image_dimensions(const ResourcePath& file, unsigned int texture_index)
	{
		return internal::texture_registry.get_image_dimensions(file, texture_index);
	}

	graphics::ImageDimensions get_image_dimensions(const graphics::BindlessTextureRef& texture)
	{
		return internal::texture_registry.get_image_dimensions(texture);
	}

	SmartReference<graphics::AnimDimensions> get_anim_dimensions(const ResourcePath& file, unsigned int texture_index)
	{
		return internal::texture_registry.get_anim_dimensions(file, texture_index);
	}

	SmartReference<graphics::AnimDimensions> get_anim_dimensions(const graphics::BindlessTextureRef& texture)
	{
		return internal::texture_registry.get_anim_dimensions(texture);
	}
}
