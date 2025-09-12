#pragma once

#include <variant>

#include "graphics/backend/basic/Textures.h"
#include "core/containers/Bijection.h"

namespace oly::reg
{
	class TextureRegistry
	{
		struct TextureKey
		{
			std::string file;
			unsigned int index;

			bool operator==(const TextureKey&) const = default;
		};

		struct TextureHash
		{
			size_t operator()(const TextureKey& k) const { return std::hash<std::string>{}(k.file) ^ std::hash<unsigned int>{}(k.index); }
		};

		std::unordered_map<TextureKey, graphics::ImageRef, TextureHash> images;
		std::unordered_map<TextureKey, graphics::AnimRef, TextureHash> anims;
		std::unordered_map<TextureKey, graphics::VectorImageRef, TextureHash> vector_images;
		
		std::unordered_map<std::string, graphics::NSVGAbstract> nsvg_abstracts;

		Bijection<TextureKey, graphics::BindlessTextureRef, TextureHash> textures;

	public:
		typedef std::variant<graphics::ImageRef, graphics::AnimRef, graphics::VectorImageRef> CPUBuffer;

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
			CPUBuffer** cpubuffer = nullptr;
			bool set_and_use = true;
		};

		struct TempSVGLoadParams
		{
			CPUBuffer** cpubuffer = nullptr;
			graphics::NSVGAbstract** abstract = nullptr;
			bool set_and_use = true;
		};

		graphics::BindlessTextureRef load_texture(const std::string& file, unsigned int texture_index = 0, LoadParams params = {});
		graphics::BindlessTextureRef load_svg_texture(const std::string& file, unsigned int texture_index = 0, SVGLoadParams params = {});
		graphics::BindlessTextureRef load_temp_texture(const std::string& file, unsigned int texture_index = 0, TempLoadParams params = {}) const;
		graphics::BindlessTextureRef load_temp_svg_texture(const std::string& file, unsigned int texture_index = 0, TempSVGLoadParams params = {}) const;

		void clear();

		glm::vec2 get_dimensions(const std::string& file, unsigned int texture_index = 0) const;
		graphics::ImageDimensions get_image_dimensions(const std::string& file, unsigned int texture_index = 0) const;
		std::weak_ptr<graphics::AnimDimensions> get_anim_dimensions(const std::string& file, unsigned int texture_index = 0) const;
		graphics::ImageRef get_image_pixel_buffer(const std::string& file, unsigned int texture_index = 0);
		graphics::AnimRef get_anim_pixel_buffer(const std::string& file, unsigned int texture_index = 0);

		glm::vec2 get_dimensions(const graphics::BindlessTextureRef& texture) const;
		graphics::ImageDimensions get_image_dimensions(const graphics::BindlessTextureRef& texture) const;
		std::weak_ptr<graphics::AnimDimensions> get_anim_dimensions(const graphics::BindlessTextureRef& texture) const;
		graphics::ImageRef get_image_pixel_buffer(const graphics::BindlessTextureRef& texture);
		graphics::AnimRef get_anim_pixel_buffer(const graphics::BindlessTextureRef& texture);

	private:
		glm::vec2 get_dimensions(const TextureKey& key) const;
		graphics::ImageDimensions get_image_dimensions(const TextureKey& key) const;
		std::weak_ptr<graphics::AnimDimensions> get_anim_dimensions(const TextureKey& key) const;
		graphics::ImageRef get_image_pixel_buffer(const TextureKey& key);
		graphics::AnimRef get_anim_pixel_buffer(const TextureKey& key);

	public:
		const graphics::NSVGAbstract& get_nsvg_abstract(const std::string& file) const;

		void free_texture(const std::string& file, unsigned int texture_index = 0);
		void free_svg_texture(const std::string& file, unsigned int texture_index = 0);
		void free_nsvg_abstract(const std::string& file);
	};
}
