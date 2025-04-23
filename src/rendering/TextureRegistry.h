#pragma once

#include <unordered_map>

#include "core/Textures.h"
#include "Loader.h"

namespace oly
{
	class TextureRegistry
	{
		typedef std::variant<rendering::ImageBindlessTextureRes, rendering::GIFBindlessTextureRes> Registree;
		static void delete_buffer(Registree& r) { std::visit([](auto&& r) {
			if constexpr (std::is_same_v<std::decay_t<decltype(r)>, rendering::ImageDimensions>)
				r.image.delete_buffer();
			else if constexpr (std::is_same_v<std::decay_t<decltype(r)>, rendering::GIFDimensions>)
				r.gif.delete_buffer();
			}, r);
		}

		std::unordered_map<std::string, Registree> reg;

	public:
		enum class TextureType
		{
			IMAGE,
			GIF
		};
	
	private:
		static rendering::BindlessTextureRes create_texture(const assets::AssetNode& node, const rendering::Image& image);
		static rendering::BindlessTextureRes create_texture(const assets::AssetNode& node, const rendering::GIF& gif);
		static void setup_texture(const rendering::BindlessTextureRes& texture, const assets::AssetNode& node, GLenum target);
		void register_gif(const assets::AssetNode& node, const rendering::GIFRes& gif);
		void register_image(const assets::AssetNode& node, const rendering::ImageRes& image);
		void load_registree(const std::string& root_dir, const assets::AssetNode& node);

	public:
		void load(const char* texture_registry_file);
		void load(const std::string& texture_registry_file) { load(texture_registry_file.c_str()); }

		rendering::BindlessTextureRes get_texture(const std::string& name) const;
		rendering::ImageDimensions get_image_dimensions(const std::string& name) const;
		std::weak_ptr<rendering::GIFDimensions> get_gif_dimensions(const std::string& name) const;
		rendering::ImageRes get_image_pixel_buffer(const std::string& name);
		rendering::GIFRes get_gif_pixel_buffer(const std::string& name);

		TextureType get_type(const std::string& name) const;

	private:
		decltype(reg)::const_iterator get_registree(const std::string& name) const;
	};
}
