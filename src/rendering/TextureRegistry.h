#pragma once

#include <unordered_map>

#include "core/Textures.h"
#include "Loader.h"

namespace oly
{
	class Context;
	class TextureRegistry
	{
		typedef std::variant<rendering::ImageBindlessTextureRes, rendering::GIFBindlessTextureRes, rendering::NSVGBindlessTextureRes> TextureRegistree;
		static void delete_buffer(TextureRegistree& r) { std::visit([](auto&& r) {
			if constexpr (std::is_same_v<std::decay_t<decltype(r)>, rendering::ImageDimensions>)
				r.image.delete_buffer();
			else if constexpr (std::is_same_v<std::decay_t<decltype(r)>, rendering::GIFDimensions>)
				r.gif.delete_buffer();
			}, r);
		}

		std::unordered_map<std::string, TextureRegistree> texture_reg;
		std::unordered_map<std::string, rendering::NSVGAbstract> nsvg_abstract_reg;

	public:
		enum class TextureType
		{
			IMAGE,
			GIF,
			NSVG
		};
	
	private:
		static rendering::BindlessTextureRes create_texture(const assets::AssetNode& node, const rendering::Image& image);
		static rendering::BindlessTextureRes create_texture(const assets::AssetNode& node, const rendering::GIF& gif);
		static rendering::BindlessTextureRes create_texture(const assets::AssetNode& node, const rendering::NSVGImageRes& image);
		static void setup_texture(const rendering::BindlessTextureRes& texture, const assets::AssetNode& node, GLenum target);
		void register_gif(const assets::AssetNode& node, const rendering::GIFRes& gif);
		void register_image(const assets::AssetNode& node, const rendering::ImageRes& image);
		void register_nsvg_image(const assets::AssetNode& node, const rendering::NSVGImageRes& image, const std::string& name);
		void load_registree(const std::string& root_dir, const assets::AssetNode& node);

		void load_nsvg_abstract(const Context* context, const std::string& root_dir, const assets::AssetNode& node);

	public:
		void load(const Context* context, const char* texture_registry_file);
		void load(const Context* context, const std::string& texture_registry_file) { load(context, texture_registry_file.c_str()); }

		rendering::BindlessTextureRes get_texture(const std::string& name) const;
		rendering::ImageDimensions get_image_dimensions(const std::string& name) const;
		std::weak_ptr<rendering::GIFDimensions> get_gif_dimensions(const std::string& name) const;
		rendering::ImageRes get_image_pixel_buffer(const std::string& name);
		rendering::GIFRes get_gif_pixel_buffer(const std::string& name);

		TextureType get_type(const std::string& name) const;
		
		const rendering::NSVGAbstract& get_nsvg_abstract(const std::string& name) const;
		float get_nsvg_scale(const std::string& name) const;

	private:
		decltype(texture_reg)::const_iterator get_registree(const std::string& name) const;
	};
}
