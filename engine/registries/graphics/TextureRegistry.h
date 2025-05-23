#pragma once

#include <variant>

#include "graphics/backend/basic/Textures.h"

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

		struct SVGTextureKey
		{
			std::string file;
			unsigned int index;
			float scale;

			bool operator==(const SVGTextureKey&) const = default;
		};

		struct SVGTextureHash
		{
			size_t operator()(const SVGTextureKey& k) const
				{ return std::hash<std::string>{}(k.file) ^ std::hash<unsigned int>{}(k.index) ^ std::hash<float>{}(k.scale); }
		};

		std::unordered_map<TextureKey, graphics::ImageRes, TextureHash> images;
		std::unordered_map<TextureKey, graphics::AnimRes, TextureHash> anims;
		std::unordered_map<TextureKey, graphics::VectorImageRes, TextureHash> vector_images;
		std::unordered_map<TextureKey, graphics::BindlessTextureRes, TextureHash> textures;
		std::unordered_map<SVGTextureKey, graphics::BindlessTextureRes, SVGTextureHash> svg_textures;
		std::unordered_map<TextureKey, graphics::NSVGAbstract, TextureHash> nsvg_abstracts;

	public:
		typedef std::variant<graphics::ImageRes, graphics::AnimRes, graphics::VectorImageRes> CPUBuffer;

		enum class ImageStorageOverride
		{
			DEFAULT,
			KEEP,
			DISCARD
		};

		struct LoadParams
		{
			unsigned int texture_index = 0;
			ImageStorageOverride storage = ImageStorageOverride::DEFAULT;
			bool set_and_use = true;
		};

		struct SVGLoadParams
		{
			unsigned int texture_index = 0;
			ImageStorageOverride abstract_storage = ImageStorageOverride::DEFAULT;
			ImageStorageOverride image_storage = ImageStorageOverride::DEFAULT;
			bool set_and_use = true;
		};

		struct TempLoadParams
		{
			unsigned int texture_index = 0;
			CPUBuffer** cpubuffer = nullptr;
			bool set_and_use = true;
		};

		struct TempSVGLoadParams
		{
			unsigned int texture_index = 0;
			CPUBuffer** cpubuffer = nullptr;
			graphics::NSVGAbstract** abstract = nullptr;
			bool set_and_use = true;
		};

		graphics::BindlessTextureRes load_texture(const std::string& file, LoadParams params = {});
		graphics::BindlessTextureRes load_svg_texture(const std::string& file, float scale = 1.0f, SVGLoadParams params = {});
		graphics::BindlessTextureRes load_temp_texture(const std::string& file, TempLoadParams params = {}) const;
		graphics::BindlessTextureRes load_temp_svg_texture(const std::string& file, float scale = 1.0f, TempSVGLoadParams params = {}) const;

		void clear();

		glm::vec2 get_dimensions(const std::string& file, unsigned int texture_index = 0);
		graphics::ImageDimensions get_image_dimensions(const std::string& file, unsigned int texture_index = 0) const;
		std::weak_ptr<graphics::AnimDimensions> get_anim_dimensions(const std::string& file, unsigned int texture_index = 0) const;
		graphics::ImageRes get_image_pixel_buffer(const std::string& file, unsigned int texture_index = 0);
		graphics::AnimRes get_anim_pixel_buffer(const std::string& file, unsigned int texture_index = 0);
		const graphics::NSVGAbstract& get_nsvg_abstract(const std::string& file, unsigned int texture_index = 0) const;

		void free_texture(const std::string& file, unsigned int texture_index = 0);
		void free_svg_texture(const std::string& file, float scale = 1.0f, unsigned int texture_index = 0);
		void free_nsvg_abstract(const std::string& file, unsigned int texture_index = 0);
	};
}
