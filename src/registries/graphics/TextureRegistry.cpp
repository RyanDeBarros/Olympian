#include "TextureRegistry.h"

#include <filesystem>

#include "core/base/Context.h"
#include "core/base/Errors.h"
#include "core/util/IO.h"
#include "registries/Loader.h"

namespace oly::reg
{
	graphics::BindlessTextureRes TextureRegistry::create_texture(const TOMLNode& node, const graphics::Image& image)
	{
		bool generate_mipmaps = node["generate mipmaps"].value<bool>().value_or(false);
		return move_shared(graphics::load_bindless_texture_2d(image, generate_mipmaps));
	}

	graphics::BindlessTextureRes TextureRegistry::create_texture(const TOMLNode& node, const graphics::Anim& anim)
	{
		bool generate_mipmaps = node["generate mipmaps"].value<bool>().value_or(false);
		return move_shared(graphics::load_bindless_texture_2d_array(anim, generate_mipmaps));
	}

	graphics::BindlessTextureRes TextureRegistry::create_texture(const TOMLNode& node, const graphics::VectorImageRes& image, const graphics::NSVGAbstract& abstract, const graphics::NSVGContext& context)
	{
		std::string generate_mipmaps = node["generate mipmaps"].value<std::string>().value_or("");
		if (generate_mipmaps == "auto")
			return move_shared(graphics::load_bindless_nsvg_texture_2d(image, true));
		else if (generate_mipmaps == "manual")
		{
			auto texture = graphics::load_bindless_nsvg_texture_2d(image, false);
			graphics::nsvg_manually_generate_mipmaps(image, abstract, context);
			return move_shared(std::move(texture));
		}
		else
			return move_shared(graphics::load_bindless_nsvg_texture_2d(image, false));
	}

	void TextureRegistry::setup_texture(const graphics::BindlessTextureRes& texture, const TOMLNode& node, GLenum target)
	{
		GLenum min_filter, mag_filter, wrap_s, wrap_t;
		if (!parse_min_filter(node, "min filter", min_filter))
			min_filter = GL_NEAREST;
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
		if (!parse_mag_filter(node, "mag filter", mag_filter))
			mag_filter = GL_NEAREST;
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
		if (parse_wrap(node, "wrap s", wrap_s))
			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
		if (parse_wrap(node, "wrap t", wrap_t))
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);

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

	void TextureRegistry::register_anim(const TOMLNode& node, const graphics::AnimRes& anim)
	{
		auto name = node["name"].value<std::string>();
		if (name)
		{
			graphics::BindlessTextureRes texture = create_texture(node, *anim);
			setup_texture(texture, node, GL_TEXTURE_2D_ARRAY);
			texture_reg[name.value()] = AnimBindlessTextureRes{ anim, std::move(texture)};
		}
	}

	void TextureRegistry::register_image(const TOMLNode& node, const graphics::ImageRes& image)
	{
		auto name = node["name"].value<std::string>();
		if (name)
		{
			graphics::BindlessTextureRes texture = create_texture(node, *image);
			setup_texture(texture, node, GL_TEXTURE_2D);
			texture_reg[name.value()] = ImageBindlessTextureRes{ image, std::move(texture) };
		}
	}

	void TextureRegistry::register_nsvg_image(const TOMLNode& node, const graphics::VectorImageRes& image, const std::string& name, const graphics::NSVGAbstract& abstract, const graphics::NSVGContext& context)
	{
		graphics::BindlessTextureRes texture = create_texture(node, image, abstract, context);
		setup_texture(texture, node, GL_TEXTURE_2D);
		texture_reg[name] = VectorBindlessTextureRes{ image, std::move(texture) };
	}

	void TextureRegistry::load_registree(const std::string& root_dir, const TOMLNode& node)
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
			graphics::SpritesheetOptions options;
			options.rows                 = (GLuint)node["rows"].value<int64_t>().value_or(1);
			options.cols                 = (GLuint)node["cols"].value<int64_t>().value_or(1);
			options.cell_width_override  = (GLuint)node["cell width override"].value<int64_t>().value_or(0);
			options.cell_height_override = (GLuint)node["cell height override"].value<int64_t>().value_or(0);
			options.delay_cs             = (int)node["delay cs"].value<int64_t>().value_or(0);
			options.row_major            = (bool)node["row major"].value<bool>().value_or(true);
			options.row_up               = (bool)node["row up"].value<bool>().value_or(true);
			graphics::AnimRes anim = std::make_shared<graphics::Anim>(image_filepath, options);
			if (texture_list)
				texture_list->for_each([this, &anim](auto&& node) { register_anim((TOMLNode)node, anim); });
			else
				register_anim(node, anim);
			
			if (!keep_pixel_buffer)
				anim->delete_buffer();
		}
		else
		{
			graphics::ImageRes image = std::make_shared<graphics::Image>(image_filepath);
			if (texture_list)
				texture_list->for_each([this, &image](auto&& node) { register_image((TOMLNode)node, image); });
			else
				register_image(node, image);

			if (!keep_pixel_buffer)
				image->delete_buffer();
		}
	}

	void TextureRegistry::load_nsvg_abstract(const std::string& root_dir, const TOMLNode& node)
	{
		auto _file = node["file"].value<std::string>();
		auto _name = node["name"].value<std::string>();
		if (!_file || !_name)
			return;

		std::string file = _file.value();
		const std::string& abstract_name = _name.value();

		std::string units = node["units"].value<std::string>().value_or("px");
		float dpi = (float)node["dpi"].value<double>().value_or(96.0);

		graphics::NSVGAbstract nsvg_abstract(root_dir + file, units.c_str(), dpi);
		
		if (auto texture_list = node["texture"].as_array())
			texture_list->for_each([this, &nsvg_abstract, &abstract_name](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto _name = node["name"].value<std::string>();
					auto _scale = node["scale"].value<double>();
					if (!_name || !_scale)
						return;

					float scale = (float)_scale.value();
					graphics::VectorImageRes image = { context::nsvg_context().rasterize_res(nsvg_abstract, scale), scale };
					register_nsvg_image((TOMLNode)node, image, _name.value(), nsvg_abstract, context::nsvg_context());

					bool keep_pixel_buffer = node["keep pixel buffer"].value<bool>().value_or(false);
					if (!keep_pixel_buffer)
						image.image->delete_buffer();
				}});
		
		bool discard = node["discard"].value<bool>().value_or(false);
		if (!discard)
			nsvg_abstract_reg.emplace(abstract_name, std::move(nsvg_abstract));
	}

	void TextureRegistry::load(const char* texture_registry_file)
	{
		auto toml = load_toml(texture_registry_file);
		auto texture_registry = toml["texture_registry"];
		if (!texture_registry)
			return;
		auto textures = texture_registry["image"].as_array();
		if (!textures)
			return;
		std::string root_dir = io::directory_of(texture_registry_file) + texture_registry["root"].value<std::string>().value_or("");
		textures->for_each([this, &root_dir](auto&& node) { load_registree(root_dir, (TOMLNode)node); });
		if (auto nsvg_abstracts = texture_registry["nsvg_abstract"].as_array())
			nsvg_abstracts->for_each([this, &root_dir](auto&& node) { load_nsvg_abstract(root_dir, (TOMLNode)node); });
	}

	void TextureRegistry::clear()
	{
		texture_reg.clear();
		nsvg_abstract_reg.clear();
	}

	graphics::BindlessTextureRes TextureRegistry::get_texture(const std::string& name) const
	{
		return std::visit([](auto&& r) -> graphics::BindlessTextureRes { return r.texture; }, get_registree(name)->second);
	}

	graphics::ImageDimensions TextureRegistry::get_image_dimensions(const std::string& name) const
	{
		return std::visit([](auto&& r) -> graphics::ImageDimensions {
			if constexpr (std::is_same_v<std::decay_t<decltype(r)>, ImageBindlessTextureRes>)
				return r.image->dim();
			else if constexpr (std::is_same_v<std::decay_t<decltype(r)>, VectorBindlessTextureRes>)
				return r.image.image->dim();
			else if constexpr (std::is_same_v<std::decay_t<decltype(r)>, AnimBindlessTextureRes>)
			{
				auto sp = r.anim->dim().lock();
				return { sp->w, sp->h, sp->cpp };
			}
			}, get_registree(name)->second);
	}

	std::weak_ptr<graphics::AnimDimensions> TextureRegistry::get_anim_dimensions(const std::string& name) const
	{
		return std::get<(size_t)TextureType::ANIM>(get_registree(name)->second).anim->dim();
	}

	graphics::ImageRes TextureRegistry::get_image_pixel_buffer(const std::string& name)
	{
		return std::get<(size_t)TextureType::IMAGE>(get_registree(name)->second).image;
	}

	graphics::AnimRes TextureRegistry::get_anim_pixel_buffer(const std::string& name)
	{
		return std::get<(size_t)TextureType::ANIM>(get_registree(name)->second).anim;
	}

	TextureRegistry::TextureType TextureRegistry::get_type(const std::string& name) const
	{
		return (TextureType)get_registree(name)->second.index();
	}

	const graphics::NSVGAbstract& TextureRegistry::get_nsvg_abstract(const std::string& name) const
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
