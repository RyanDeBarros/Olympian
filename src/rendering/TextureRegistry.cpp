#include "TextureRegistry.h"

#include <filesystem>

#include "../Olympian.h"
#include "util/Errors.h"
#include "util/IO.h"

namespace oly
{
	rendering::BindlessTextureRes TextureRegistry::create_texture(const assets::AssetNode& node, const rendering::Image& image)
	{
		bool generate_mipmaps = node["generate mipmaps"].value<bool>().value_or(false);
		return rendering::load_bindless_texture_2d(image, generate_mipmaps);
	}

	rendering::BindlessTextureRes TextureRegistry::create_texture(const assets::AssetNode& node, const rendering::Anim& anim)
	{
		bool generate_mipmaps = node["generate mipmaps"].value<bool>().value_or(false);
		return rendering::load_bindless_texture_2d_array(anim, generate_mipmaps);
	}

	rendering::BindlessTextureRes TextureRegistry::create_texture(const assets::AssetNode& node, const rendering::VectorImageRes& image, const rendering::NSVGAbstract& abstract, const rendering::NSVGContext& context)
	{
		std::string generate_mipmaps = node["generate mipmaps"].value<std::string>().value_or("");
		if (generate_mipmaps == "auto")
			return rendering::load_bindless_nsvg_texture_2d(image, true);
		else if (generate_mipmaps == "manual")
		{
			auto texture = rendering::load_bindless_nsvg_texture_2d(image, false);
			rendering::nsvg_manually_generate_mipmaps(image, abstract, context);
			return texture;
		}
		else
			return rendering::load_bindless_nsvg_texture_2d(image, false);
	}

	void TextureRegistry::setup_texture(const rendering::BindlessTextureRes& texture, const assets::AssetNode& node, GLenum target)
	{
		std::string min_filter = node["min filter"].value<std::string>().value_or("nearest");
		std::string mag_filter = node["mag filter"].value<std::string>().value_or("nearest");

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
				texture->set_and_use_handle();
			else
				texture->set_handle();
		}
	}

	void TextureRegistry::register_anim(const assets::AssetNode& node, const rendering::AnimRes& anim)
	{
		auto name = node["name"].value<std::string>();
		if (name)
		{
			rendering::BindlessTextureRes texture = create_texture(node, *anim);
			setup_texture(texture, node, GL_TEXTURE_2D_ARRAY);
			texture_reg[name.value()] = rendering::AnimBindlessTextureRes{ anim, std::move(texture)};
		}
	}

	void TextureRegistry::register_image(const assets::AssetNode& node, const rendering::ImageRes& image)
	{
		auto name = node["name"].value<std::string>();
		if (name)
		{
			rendering::BindlessTextureRes texture = create_texture(node, *image);
			setup_texture(texture, node, GL_TEXTURE_2D);
			texture_reg[name.value()] = rendering::ImageBindlessTextureRes{ image, std::move(texture) };
		}
	}

	void TextureRegistry::register_nsvg_image(const assets::AssetNode& node, const rendering::VectorImageRes& image, const std::string& name, const rendering::NSVGAbstract& abstract, const rendering::NSVGContext& context)
	{
		rendering::BindlessTextureRes texture = create_texture(node, image, abstract, context);
		setup_texture(texture, node, GL_TEXTURE_2D);
		texture_reg[name] = rendering::VectorBindlessTextureRes{ image, std::move(texture) };
	}

	void TextureRegistry::load_registree(const std::string& root_dir, const assets::AssetNode& node)
	{
		auto _file = node["file"].value<std::string>();
		if (!_file)
			return;

		std::string file = _file.value();
		auto texture_list = node["texture"].as_array();
		bool keep_pixel_buffer = node["keep pixel buffer"].value<bool>().value_or(false);
		bool anim = node["anim"].value<bool>().value_or(false);

		const std::string _image_filepath = root_dir + file;
		const char* image_filepath = _image_filepath.c_str();
		if (anim)
		{
			rendering::SpritesheetOptions options;
			options.rows                 = (GLuint)node["rows"].value<int64_t>().value_or(1);
			options.cols                 = (GLuint)node["cols"].value<int64_t>().value_or(1);
			options.cell_width_override  = (GLuint)node["cell width override"].value<int64_t>().value_or(0);
			options.cell_height_override = (GLuint)node["cell height override"].value<int64_t>().value_or(0);
			options.delay_cs             = (int)node["delay cs"].value<int64_t>().value_or(0);
			options.row_major            = (bool)node["row major"].value<bool>().value_or(true);
			options.row_up               = (bool)node["row up"].value<bool>().value_or(true);
			rendering::AnimRes anim = std::make_shared<rendering::Anim>(image_filepath, options);
			if (texture_list)
				texture_list->for_each([this, &anim](auto&& node) { register_anim((assets::AssetNode)node, anim); });
			else
				register_anim(node, anim);
			
			if (!keep_pixel_buffer)
				anim->delete_buffer();
		}
		else
		{
			rendering::ImageRes image = std::make_shared<rendering::Image>(image_filepath);
			if (texture_list)
				texture_list->for_each([this, &image](auto&& node) { register_image((assets::AssetNode)node, image); });
			else
				register_image(node, image);

			if (!keep_pixel_buffer)
				image->delete_buffer();
		}
	}

	void TextureRegistry::load_nsvg_abstract(const Context* context, const std::string& root_dir, const assets::AssetNode& node)
	{
		auto _file = node["file"].value<std::string>();
		auto _name = node["name"].value<std::string>();
		if (!_file || !_name)
			return;

		std::string file = _file.value();
		const std::string& abstract_name = _name.value();

		std::string units = node["units"].value<std::string>().value_or("px");
		float dpi = (float)node["dpi"].value<double>().value_or(96.0);

		rendering::NSVGAbstract nsvg_abstract(root_dir + file, units.c_str(), dpi);
		
		if (auto texture_list = node["texture"].as_array())
			texture_list->for_each([this, context, &nsvg_abstract, &abstract_name](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto _name = node["name"].value<std::string>();
					auto _scale = node["scale"].value<double>();
					if (!_name || !_scale)
						return;

					float scale = (float)_scale.value();
					rendering::VectorImageRes image = { context->nsvg_context().rasterize_res(nsvg_abstract, scale), scale };
					register_nsvg_image((assets::AssetNode)node, image, _name.value(), nsvg_abstract, context->nsvg_context());

					bool keep_pixel_buffer = node["keep pixel buffer"].value<bool>().value_or(false);
					if (!keep_pixel_buffer)
						image.image->delete_buffer();
				}});
		
		bool discard = node["discard"].value<bool>().value_or(false);
		if (!discard)
			nsvg_abstract_reg.emplace(abstract_name, std::move(nsvg_abstract));
	}

	void TextureRegistry::load(const Context* context, const char* texture_registry_file)
	{
		auto toml = assets::load_toml(texture_registry_file);
		auto texture_registry = toml["texture_registry"];
		if (!texture_registry)
			return;
		auto textures = texture_registry["image"].as_array();
		if (!textures)
			return;
		std::string root_dir = std::filesystem::path(texture_registry_file).parent_path().string() + "/" + texture_registry["root"].value<std::string>().value_or("");
		textures->for_each([this, &root_dir](auto&& node) { load_registree(root_dir, (assets::AssetNode)node); });
		if (auto nsvg_abstracts = texture_registry["nsvg_abstract"].as_array())
			nsvg_abstracts->for_each([this, context, &root_dir](auto&& node) { load_nsvg_abstract(context, root_dir, (assets::AssetNode)node); });
	}

	void TextureRegistry::clear()
	{
		texture_reg.clear();
		nsvg_abstract_reg.clear();
	}

	rendering::BindlessTextureRes oly::TextureRegistry::get_texture(const std::string& name) const
	{
		return std::visit([](auto&& r) -> rendering::BindlessTextureRes { return r.texture; }, get_registree(name)->second);
	}

	rendering::ImageDimensions oly::TextureRegistry::get_image_dimensions(const std::string& name) const
	{
		const auto& r = get_registree(name)->second;
		if (r.index() == (size_t)TextureType::IMAGE)
			return std::get<(size_t)TextureType::IMAGE>(get_registree(name)->second).image->dim();
		else
			return std::get<(size_t)TextureType::NSVG>(get_registree(name)->second).image.image->dim();
	}

	std::weak_ptr<rendering::AnimDimensions> oly::TextureRegistry::get_anim_dimensions(const std::string& name) const
	{
		return std::get<(size_t)TextureType::ANIM>(get_registree(name)->second).anim->dim();
	}

	rendering::ImageRes TextureRegistry::get_image_pixel_buffer(const std::string& name)
	{
		return std::get<(size_t)TextureType::IMAGE>(get_registree(name)->second).image;
	}

	rendering::AnimRes TextureRegistry::get_anim_pixel_buffer(const std::string& name)
	{
		return std::get<(size_t)TextureType::ANIM>(get_registree(name)->second).anim;
	}

	TextureRegistry::TextureType TextureRegistry::get_type(const std::string& name) const
	{
		return (TextureType)get_registree(name)->second.index();
	}

	const rendering::NSVGAbstract& TextureRegistry::get_nsvg_abstract(const std::string& name) const
	{
		auto it = nsvg_abstract_reg.find(name);
		if (it == nsvg_abstract_reg.end())
			throw Error(ErrorCode::UNREGISTERED_NSVG_ABSTRACT);
		return it->second;
	}

	float TextureRegistry::get_nsvg_scale(const std::string& name) const
	{
		return std::get<(size_t)TextureType::NSVG>(get_registree(name)->second).image.scale;
	}

	decltype(TextureRegistry::texture_reg)::const_iterator TextureRegistry::get_registree(const std::string& name) const
	{
		auto it = texture_reg.find(name);
		if (it == texture_reg.end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return it;
	}
}
