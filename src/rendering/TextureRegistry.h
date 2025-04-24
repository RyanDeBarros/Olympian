#pragma once

#include <unordered_map>

#include "core/Textures.h"
#include "Loader.h"

namespace oly
{
	class Context;
	class TextureRegistry
	{
		typedef std::variant<rendering::ImageBindlessTextureRes, rendering::AnimBindlessTextureRes, rendering::VectorBindlessTextureRes> TextureRegistree;
		static void delete_buffer(TextureRegistree& r) { std::visit([](auto&& r) {
			if constexpr (std::is_same_v<std::decay_t<decltype(r)>, rendering::ImageDimensions>)
				r.image.delete_buffer();
			else if constexpr (std::is_same_v<std::decay_t<decltype(r)>, rendering::AnimDimensions>)
				r.anim.delete_buffer();
			}, r);
		}

		std::unordered_map<std::string, TextureRegistree> texture_reg;
		std::unordered_map<std::string, rendering::NSVGAbstract> nsvg_abstract_reg;

	public:
		enum class TextureType
		{
			IMAGE,
			ANIM,
			NSVG
		};
	
	private:
		static rendering::BindlessTextureRes create_texture(const assets::AssetNode& node, const rendering::Image& image);
		static rendering::BindlessTextureRes create_texture(const assets::AssetNode& node, const rendering::Anim& anim);
		static rendering::BindlessTextureRes create_texture(const assets::AssetNode& node, const rendering::VectorImageRes& image, const rendering::NSVGAbstract& abstract, const rendering::NSVGContext& context);
		static void setup_texture(const rendering::BindlessTextureRes& texture, const assets::AssetNode& node, GLenum target);
		void register_anim(const assets::AssetNode& node, const rendering::AnimRes& anim);
		void register_image(const assets::AssetNode& node, const rendering::ImageRes& image);
		void register_nsvg_image(const assets::AssetNode& node, const rendering::VectorImageRes& image, const std::string& name, const rendering::NSVGAbstract& abstract, const rendering::NSVGContext& context);
		void load_registree(const std::string& root_dir, const assets::AssetNode& node);

		void load_nsvg_abstract(const Context* context, const std::string& root_dir, const assets::AssetNode& node);

	public:
		void load(const Context* context, const char* texture_registry_file);
		void load(const Context* context, const std::string& texture_registry_file) { load(context, texture_registry_file.c_str()); }
		void clear();

		rendering::BindlessTextureRes get_texture(const std::string& name) const;
		rendering::ImageDimensions get_image_dimensions(const std::string& name) const;
		std::weak_ptr<rendering::AnimDimensions> get_anim_dimensions(const std::string& name) const;
		rendering::ImageRes get_image_pixel_buffer(const std::string& name);
		rendering::AnimRes get_anim_pixel_buffer(const std::string& name);

		TextureType get_type(const std::string& name) const;
		
		const rendering::NSVGAbstract& get_nsvg_abstract(const std::string& name) const;
		float get_nsvg_scale(const std::string& name) const;

	private:
		decltype(texture_reg)::const_iterator get_registree(const std::string& name) const;
	};
}
