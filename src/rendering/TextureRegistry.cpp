#include "TextureRegistry.h"

#include <filesystem>

#include "util/Errors.h"

namespace oly
{
	void TextureRegistry::load_registree(const std::string& root_dir, const assets::AssetNode& node)
	{
		auto _name = node["name"].value<std::string>();
		auto _file = node["file"].value<std::string>();
		auto _min_filter = node["min filter"].value<std::string>();
		auto _mag_filter = node["mag filter"].value<std::string>();
		if (!_name || !_file || !_min_filter || !_mag_filter)
			return;

		std::string name = _name.value();
		std::string file = _file.value();
		std::string min_filter = _min_filter.value();
		std::string mag_filter = _mag_filter.value();
		bool generate_mipmaps = node["generate mipmaps"].value<bool>().value_or(false);
		Registree r;

		std::string type = node["type"].value<std::string>().value_or("");
		GLenum target;
		if (type == "gif")
		{
			rendering::GIFBindlessTextureRes g = oly::rendering::load_bindless_texture_2d_array(root_dir + file, generate_mipmaps);
			r = g;
			target = GL_TEXTURE_2D_ARRAY;
		}
		// TODO else if (type == "spritesheet")
		// TODO else if (type == "svg")
		else
		{
			rendering::ImageBindlessTextureRes i = oly::rendering::load_bindless_texture_2d(root_dir + file, generate_mipmaps);
			r = i;
			target = GL_TEXTURE_2D;
		}

		if (min_filter == "nearest")
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		else if (min_filter == "linear")
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		else if (min_filter == "nearest mipmap nearest")
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		else if (min_filter == "nearest mipmap linear")
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		else if (min_filter == "linear mipmap nearest")
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		else if (min_filter == "linear mipmap linear")
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		else
			throw Error(ErrorCode::LOAD_ASSET);

		if (mag_filter == "nearest")
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		else if (mag_filter == "linear")
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		else
			throw Error(ErrorCode::LOAD_ASSET);

		if (auto _wrap_s = node["wrap s"].value<std::string>())
		{
			std::string wrap_s = _wrap_s.value();
			if (wrap_s == "clamp to edge")
				glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			else if (wrap_s == "clamp to border")
				glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			else if (wrap_s == "mirrored repeat")
				glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			else if (wrap_s == "repeat")
				glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
			else if (wrap_s == "mirror clamp to edge")
				glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE);
		}
		if (auto _wrap_t = node["wrap t"].value<std::string>())
		{
			std::string wrap_t = _wrap_t.value();
			if (wrap_t == "clamp to edge")
				glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			else if (wrap_t == "clamp to border")
				glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			else if (wrap_t == "mirrored repeat")
				glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			else if (wrap_t == "repeat")
				glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
			else if (wrap_t == "mirror clamp to edge")
				glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_EDGE);
		}

		bool dont_set = node["don't set"].value<bool>().value_or(false);
		if (!dont_set)
		{
			bool auto_use = node["auto-use"].value<bool>().value_or(false);
			if (auto_use)
				texture_res(r)->set_and_use_handle();
			else
				texture_res(r)->set_handle();
		}

		bool keep_pixel_buffer = node["keep pixel buffer"].value<bool>().value_or(false);
		if (!keep_pixel_buffer)
			delete_buffer(r);

		reg[name] = r;
	}

	void TextureRegistry::load(const char* texture_registry_file)
	{
		auto toml = assets::load_toml(texture_registry_file);
		auto texture_registry = toml["texture_registry"];
		if (!texture_registry)
			return;
		auto textures = texture_registry["texture"].as_array();
		if (!textures)
			return;
		std::string root_dir = std::filesystem::path(texture_registry_file).parent_path().string() + "/" + texture_registry["root"].value<std::string>().value_or("");
		textures->for_each([this, &root_dir](auto&& node) {
			load_registree(root_dir, (assets::AssetNode)node);
			});
	}

	rendering::BindlessTextureRes oly::TextureRegistry::get_texture(const std::string& name) const
	{
		return texture_res(get_registree(name)->second);
	}

	rendering::ImageDimensions oly::TextureRegistry::get_image_dimensions(const std::string& name) const
	{
		return std::get<(size_t)TextureType::IMAGE>(get_registree(name)->second).image->dim();
	}

	std::weak_ptr<rendering::GIFDimensions> oly::TextureRegistry::get_gif_dimensions(const std::string& name) const
	{
		return std::get<(size_t)TextureType::GIF>(get_registree(name)->second).gif->dim();
	}

	rendering::ImageRes TextureRegistry::get_image_pixel_buffer(const std::string& name)
	{
		return std::get<(size_t)TextureType::IMAGE>(get_registree(name)->second).image;
	}

	rendering::GIFRes TextureRegistry::get_gif_pixel_buffer(const std::string& name)
	{
		return std::get<(size_t)TextureType::GIF>(get_registree(name)->second).gif;
	}

	TextureRegistry::TextureType TextureRegistry::get_type(const std::string& name) const
	{
		return (TextureType)get_registree(name)->second.index();
	}

	decltype(TextureRegistry::reg)::const_iterator TextureRegistry::get_registree(const std::string& name) const
	{
		auto it = reg.find(name);
		if (it == reg.end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return it;
	}
}
