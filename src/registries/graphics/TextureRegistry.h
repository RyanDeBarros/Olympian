#pragma once

#include <unordered_map>
#include <string>
#include <variant>

#include "graphics/backend/basic/Textures.h"

#include "registries/Loader.h"

namespace oly
{
	class TextureRegistry
	{
		struct ImageBindlessTextureRes
		{
			rendering::ImageRes image;
			rendering::BindlessTextureRes texture;
		};
		struct AnimBindlessTextureRes
		{
			rendering::AnimRes anim;
			rendering::BindlessTextureRes texture;
		};
		struct VectorBindlessTextureRes
		{
			rendering::VectorImageRes image;
			rendering::BindlessTextureRes texture;
		};

		typedef std::variant<ImageBindlessTextureRes, AnimBindlessTextureRes, VectorBindlessTextureRes> TextureRegistree;
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
		static rendering::BindlessTextureRes create_texture(const TOMLNode& node, const rendering::Image& image);
		static rendering::BindlessTextureRes create_texture(const TOMLNode& node, const rendering::Anim& anim);
		static rendering::BindlessTextureRes create_texture(const TOMLNode& node, const rendering::VectorImageRes& image, const rendering::NSVGAbstract& abstract, const rendering::NSVGContext& context);
		static void setup_texture(const rendering::BindlessTextureRes& texture, const TOMLNode& node, GLenum target);
		void register_anim(const TOMLNode& node, const rendering::AnimRes& anim);
		void register_image(const TOMLNode& node, const rendering::ImageRes& image);
		void register_nsvg_image(const TOMLNode& node, const rendering::VectorImageRes& image, const std::string& name, const rendering::NSVGAbstract& abstract, const rendering::NSVGContext& context);
		void load_registree(const std::string& root_dir, const TOMLNode& node);

		void load_nsvg_abstract(const std::string& root_dir, const TOMLNode& node);

	public:
		void load(const char* texture_registry_file);
		void load(const std::string& texture_registry_file) { load(texture_registry_file.c_str()); }
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
