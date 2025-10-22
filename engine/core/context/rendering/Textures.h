#pragma once

#include "graphics/backend/basic/Textures.h"
#include "core/types/Variant.h"

namespace oly::context
{
	namespace internal
	{
		extern void terminate_textures();
	}

	extern graphics::NSVGContext& nsvg_context();

	extern void sync_texture_handle(const graphics::BindlessTextureRef& texture);

	namespace tex
	{
		typedef Variant<graphics::ImageRef, graphics::AnimRef, graphics::VectorImageRef> CPUBuffer;

		enum class ImageStorageOverride
		{
			DEFAULT,
			KEEP,
			DISCARD
		};

		struct LoadParams
		{
			ImageStorageOverride storage = ImageStorageOverride::DEFAULT;
			bool set_and_use = true;
		};

		struct SVGLoadParams
		{
			ImageStorageOverride abstract_storage = ImageStorageOverride::DEFAULT;
			ImageStorageOverride image_storage = ImageStorageOverride::DEFAULT;
			bool set_and_use = true;
		};

		struct TempLoadParams
		{
			CPUBuffer* cpubuffer = nullptr;
			bool set_and_use = true;
		};

		struct TempSVGLoadParams
		{
			CPUBuffer* cpubuffer = nullptr;
			SmartReference<graphics::NSVGAbstract>* abstract = nullptr;
			bool set_and_use = true;
		};
	}

	extern graphics::BindlessTextureRef load_texture(const ResourcePath& file, unsigned int texture_index = 0, tex::LoadParams params = {});
	extern graphics::BindlessTextureRef load_svg_texture(const ResourcePath& file, unsigned int texture_index = 0, tex::SVGLoadParams params = {});
	extern graphics::BindlessTextureRef load_temp_texture(const ResourcePath& file, unsigned int texture_index = 0, tex::TempLoadParams params = {});
	extern graphics::BindlessTextureRef load_temp_svg_texture(const ResourcePath& file, unsigned int texture_index = 0, tex::TempSVGLoadParams params = {});
	
	extern glm::vec2 get_texture_dimensions(const ResourcePath& file, unsigned int texture_index = 0);
	extern graphics::ImageDimensions get_image_dimensions(const ResourcePath& file, unsigned int texture_index = 0);
	extern SmartReference<graphics::AnimDimensions> get_anim_dimensions(const ResourcePath& file, unsigned int texture_index = 0);
	extern graphics::ImageRef get_image_pixel_buffer(const ResourcePath& file, unsigned int texture_index = 0);
	extern graphics::AnimRef get_anim_pixel_buffer(const ResourcePath& file, unsigned int texture_index = 0);

	extern glm::vec2 get_texture_dimensions(const graphics::BindlessTextureRef& texture);
	extern graphics::ImageDimensions get_image_dimensions(const graphics::BindlessTextureRef& texture);
	extern SmartReference<graphics::AnimDimensions> get_anim_dimensions(const graphics::BindlessTextureRef& texture);
	extern graphics::ImageRef get_image_pixel_buffer(const graphics::BindlessTextureRef& texture);
	extern graphics::AnimRef get_anim_pixel_buffer(const graphics::BindlessTextureRef& texture);

	extern const graphics::NSVGAbstract& get_nsvg_abstract(const ResourcePath& file);

	void free_texture(const ResourcePath& file, unsigned int texture_index = 0);
	void free_svg_texture(const ResourcePath& file, unsigned int texture_index = 0);
	void free_nsvg_abstract(const ResourcePath& file);
}
