#pragma once

#include <unordered_map>
#include <string>
#include <variant>

#include "graphics/backend/basic/Textures.h"

#include "external/TOML.h"

namespace oly::reg
{
	class TextureRegistry
	{
		struct ImageBindlessTextureRes
		{
			graphics::ImageRes image;
			graphics::BindlessTextureRes texture;
		};
		struct AnimBindlessTextureRes
		{
			graphics::AnimRes anim;
			graphics::BindlessTextureRes texture;
		};
		struct VectorBindlessTextureRes
		{
			graphics::VectorImageRes image;
			graphics::BindlessTextureRes texture;
		};

		typedef std::variant<ImageBindlessTextureRes, AnimBindlessTextureRes, VectorBindlessTextureRes> TextureRegistree;
		static void delete_buffer(TextureRegistree& r) { std::visit([](auto&& r) {
			if constexpr (std::is_same_v<std::decay_t<decltype(r)>, graphics::ImageDimensions>)
				r.image.delete_buffer();
			else if constexpr (std::is_same_v<std::decay_t<decltype(r)>, graphics::AnimDimensions>)
				r.anim.delete_buffer();
			}, r);
		}

		std::unordered_map<std::string, TextureRegistree> texture_reg;
		std::unordered_map<std::string, graphics::NSVGAbstract> nsvg_abstract_reg;

	public:
		enum class TextureType
		{
			IMAGE,
			ANIM,
			NSVG
		};
	
	private:
		static graphics::BindlessTextureRes create_texture(const TOMLNode& node, const graphics::Image& image);
		static graphics::BindlessTextureRes create_texture(const TOMLNode& node, const graphics::Anim& anim);
		static graphics::BindlessTextureRes create_texture(const TOMLNode& node, const graphics::VectorImageRes& image, const graphics::NSVGAbstract& abstract, const graphics::NSVGContext& context);
		static void setup_texture(const graphics::BindlessTextureRes& texture, const TOMLNode& node, GLenum target);
		void register_anim(const TOMLNode& node, const graphics::AnimRes& anim);
		void register_image(const TOMLNode& node, const graphics::ImageRes& image);
		void register_nsvg_image(const TOMLNode& node, const graphics::VectorImageRes& image, const std::string& name, const graphics::NSVGAbstract& abstract, const graphics::NSVGContext& context);
		void load_registree(const std::string& root_dir, const TOMLNode& node);

		void load_nsvg_abstract(const std::string& root_dir, const TOMLNode& node);

	public:
		void load(const char* texture_registry_file);
		void load(const std::string& texture_registry_file) { load(texture_registry_file.c_str()); }
		void clear();

		graphics::BindlessTextureRes get_texture(const std::string& name) const;
		graphics::ImageDimensions get_image_dimensions(const std::string& name) const;
		std::weak_ptr<graphics::AnimDimensions> get_anim_dimensions(const std::string& name) const;
		graphics::ImageRes get_image_pixel_buffer(const std::string& name);
		graphics::AnimRes get_anim_pixel_buffer(const std::string& name);

		TextureType get_type(const std::string& name) const;
		
		const graphics::NSVGAbstract& get_nsvg_abstract(const std::string& name) const;
		float get_nsvg_scale(const std::string& name) const;

	private:
		decltype(texture_reg)::const_iterator get_registree(const std::string& name) const;
	};
}
