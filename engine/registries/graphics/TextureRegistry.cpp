#include "TextureRegistry.h"

#include "core/context/Context.h"
#include "core/context/rendering/Textures.h"

#include "core/types/Meta.h"
#include "core/util/Logger.h"

#include "registries/Loader.h"

namespace oly::reg
{
	static void setup_texture(graphics::BindlessTexture& texture, const TOMLNode& node, GLenum target, bool set_and_use)
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

		if (set_and_use)
			texture.set_and_use_handle();
	}

	static graphics::BindlessTextureRef load_image(const graphics::Image& image, const TOMLNode& node, bool set_and_use)
	{
		graphics::BindlessTexture texture = graphics::load_bindless_texture_2d(image, node["generate mipmaps"].value<bool>().value_or(false));
		setup_texture(texture, node, GL_TEXTURE_2D, set_and_use);
		return graphics::BindlessTextureRef(std::move(texture));
	}

	static graphics::BindlessTextureRef load_anim(const graphics::Anim& anim, const TOMLNode& node, bool set_and_use)
	{
		graphics::BindlessTexture texture = graphics::load_bindless_texture_2d_array(anim, node["generate mipmaps"].value<bool>().value_or(false));
		setup_texture(texture, node, GL_TEXTURE_2D_ARRAY, set_and_use);
		return graphics::BindlessTextureRef(std::move(texture));
	}

	static graphics::BindlessTextureRef load_svg(const graphics::NSVGAbstract& abstract, const graphics::VectorImageRef& image, const TOMLNode& node, bool set_and_use)
	{
		graphics::BindlessTextureRef texture;
		std::string generate_mipmaps = node["generate mipmaps"].value<std::string>().value_or("off");
		if (generate_mipmaps == "auto")
			texture = graphics::BindlessTextureRef(graphics::load_bindless_nsvg_texture_2d(image, true));
		else
		{
			texture = graphics::BindlessTextureRef(graphics::load_bindless_nsvg_texture_2d(image, false));
			if (generate_mipmaps == "manual")
				graphics::nsvg_manually_generate_mipmaps(image, abstract, context::nsvg_context());
		}
		setup_texture(*texture, node, GL_TEXTURE_2D, set_and_use);
		return texture;
	}

	static void load_texture_node(const std::string& file, toml::parse_result& toml, TOMLNode& texture_node, size_t texture_index)
	{
		std::string oly_file = file + ".oly";
		try
		{
			toml = load_toml(oly_file);
			auto texture_array = toml["texture"].as_array();
			if (texture_array && !texture_array->empty())
			{
				texture_index = glm::clamp(texture_index, size_t(0), texture_array->size() - size_t(1));
				texture_node = TOMLNode(*texture_array->get(texture_index));
			}
			else
				LOG.warning() << "texture table does not exist in texture import file: " << oly_file << LOG.nl;
		}
		catch (Error e)
		{
			if (e.code == ErrorCode::TOML_PARSE)
				LOG.warning() << "cannot parse texture import file: " << oly_file << LOG.nl;
			else
				throw e;
		}
	}

	static bool should_store(const TOMLNode& texture_node, const char* storage_key, TextureRegistry::ImageStorageOverride storage_override)
	{
		if (storage_override == TextureRegistry::ImageStorageOverride::DISCARD)
			return false;
		else if (storage_override == TextureRegistry::ImageStorageOverride::KEEP)
			return true;
		else
			return texture_node[storage_key].value<std::string>().value_or("discard") == "keep";
	}

	static graphics::SpritesheetOptions parse_spritesheet_options(const TOMLNode& texture_node)
	{
		graphics::SpritesheetOptions options;
		options.rows                 = (GLuint)texture_node["rows"                ].value<int64_t>().value_or(1);
		options.cols                 = (GLuint)texture_node["cols"                ].value<int64_t>().value_or(1);
		options.cell_width_override  = (GLuint)texture_node["cell width override" ].value<int64_t>().value_or(0);
		options.cell_height_override = (GLuint)texture_node["cell height override"].value<int64_t>().value_or(0);
		options.delay_cs             = (int)   texture_node["delay cs"            ].value<int64_t>().value_or(0);
		options.row_major            = (bool)  texture_node["row major"           ].value<bool>().value_or(true);
		options.row_up               = (bool)  texture_node["row up"              ].value<bool>().value_or(true);
		return options;
	}

	graphics::BindlessTextureRef TextureRegistry::load_texture(const std::string& file, LoadParams params)
	{
		if (file.ends_with(".svg"))
		{
			SVGLoadParams svg_params{
				.texture_index = params.texture_index,
				.abstract_storage = ImageStorageOverride::DEFAULT,
				.image_storage = params.storage,
				.set_and_use = params.set_and_use
			};
			svg_params.texture_index = params.texture_index;
			return load_svg_texture(file, 1.0f, svg_params);
		}
		
		TextureKey tkey { file, params.texture_index };
		auto it = textures.find(tkey);
		if (it != textures.end())
			return it->second;

		std::string full_path = context::resource_file(file);
		toml::parse_result toml;
		TOMLNode texture_node;
		load_texture_node(full_path, toml, texture_node, params.texture_index);

		bool store_buffer = should_store(texture_node, "storage", params.storage);

		graphics::BindlessTextureRef texture;

		if (file.ends_with(".gif"))
		{
			graphics::Anim anim(full_path.c_str());
			texture = load_anim(anim, texture_node, params.set_and_use);
			if (!store_buffer)
				anim.delete_buffer();
			anims[tkey] = graphics::AnimRef(std::move(anim));
		}
		else
		{
			if (texture_node["anim"].value<bool>().value_or(false))
			{
				graphics::Anim anim(full_path.c_str(), parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (!store_buffer)
					anim.delete_buffer();
				anims[tkey] = graphics::AnimRef(std::move(anim));
			}
			else
			{
				graphics::Image image(full_path.c_str());
				texture = load_image(image, texture_node, params.set_and_use);
				if (!store_buffer)
					image.delete_buffer();
				images[tkey] = graphics::ImageRef(std::move(image));
			}
		}

		textures[tkey] = texture;
		return texture;
	}
	
	static graphics::NSVGAbstract create_abstract(const TOMLNode& texture_node, const std::string& full_path)
	{
		return graphics::NSVGAbstract(full_path, texture_node["svg units"].value<std::string>().value_or("px").c_str(), (float)texture_node["svg dpi"].value<double>().value_or(96.0f));
	}

	graphics::BindlessTextureRef TextureRegistry::load_svg_texture(const std::string& file, float scale, SVGLoadParams params)
	{
		if (!file.ends_with(".svg"))
			LOG.warning() << "attempting to load non-svg file as svg texture: " << file << LOG.nl;

		SVGTextureKey svg_tkey{ file, params.texture_index, scale };
		auto it = svg_textures.find(svg_tkey);
		if (it != svg_textures.end())
			return it->second;

		TextureKey tkey{ file, params.texture_index };
		std::string full_path = context::resource_file(file);
		toml::parse_result toml;
		TOMLNode texture_node;
		load_texture_node(full_path, toml, texture_node, params.texture_index);

		bool store_abstract = should_store(texture_node, "abstract storage", params.abstract_storage);
		bool store_image = should_store(texture_node, "image storage", params.image_storage);

		graphics::BindlessTextureRef texture;

		if (texture_node["anim"].value<bool>().value_or(false))
		{
			auto ait = nsvg_abstracts.find(tkey);
			if (ait != nsvg_abstracts.end())
			{
				graphics::Anim anim(ait->second, scale, parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (!store_image)
					anim.delete_buffer();
				anims[tkey] = graphics::AnimRef(std::move(anim));
			}
			else
			{
				graphics::NSVGAbstract abstract = create_abstract(texture_node, full_path);
				graphics::Anim anim(abstract, scale, parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (!store_image)
					anim.delete_buffer();
				anims[tkey] = graphics::AnimRef(std::move(anim));
				if (store_abstract)
					nsvg_abstracts.emplace(file, std::move(abstract));
			}
		}
		else
		{
			auto ait = nsvg_abstracts.find(tkey);
			if (ait != nsvg_abstracts.end())
			{
				graphics::VectorImageRef image;
				image.scale = scale;
				image.image = context::nsvg_context().rasterize_res(ait->second, scale);
				texture = load_svg(ait->second, image, texture_node, params.set_and_use);
				if (!store_image)
					image.image->delete_buffer();
				vector_images[tkey] = image;
			}
			else
			{
				graphics::NSVGAbstract abstract = create_abstract(texture_node, full_path);
				graphics::VectorImageRef image;
				image.scale = scale;
				image.image = context::nsvg_context().rasterize_res(abstract, scale);
				texture = load_svg(abstract, image, texture_node, params.set_and_use);
				if (!store_image)
					image.image->delete_buffer();
				vector_images[tkey] = image;
				if (store_abstract)
					nsvg_abstracts.emplace(file, std::move(abstract));
			}
		}

		svg_textures[svg_tkey] = texture;
		return texture;
	}
	
	graphics::BindlessTextureRef TextureRegistry::load_temp_texture(const std::string& file, TempLoadParams params) const
	{
		if (file.ends_with(".svg"))
		{
			TempSVGLoadParams svg_params{
				.texture_index = params.texture_index,
				.cpubuffer = params.cpubuffer,
				.abstract = nullptr,
				.set_and_use = params.set_and_use
			};
			return load_temp_svg_texture(file, 1.0f, svg_params);
		}

		toml::parse_result toml;
		TOMLNode texture_node;
		load_texture_node(file, toml, texture_node, params.texture_index);

		graphics::BindlessTextureRef texture;

		if (file.ends_with(".gif"))
		{
			graphics::Anim anim(file.c_str());
			texture = load_anim(anim, texture_node, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = new CPUBuffer(graphics::AnimRef(std::move(anim)));
		}
		else
		{
			if (texture_node["anim"].value<bool>().value_or(false))
			{
				graphics::Anim anim(file.c_str(), parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (params.cpubuffer)
					*params.cpubuffer = new CPUBuffer(graphics::AnimRef(std::move(anim)));
			}
			else
			{
				graphics::Image image(file.c_str());
				texture = load_image(image, texture_node, params.set_and_use);
				if (params.cpubuffer)
					*params.cpubuffer = new CPUBuffer(graphics::ImageRef(std::move(image)));
			}
		}

		return texture;
	}
	
	graphics::BindlessTextureRef TextureRegistry::load_temp_svg_texture(const std::string& file, float scale, TempSVGLoadParams params) const
	{
		if (!file.ends_with(".svg"))
			LOG.warning() << "attempting to load non-svg file as svg texture: " << file << LOG.nl;

		toml::parse_result toml;
		TOMLNode texture_node;
		load_texture_node(file, toml, texture_node, params.texture_index);

		graphics::BindlessTextureRef texture;

		if (texture_node["anim"].value<bool>().value_or(false))
		{
			graphics::NSVGAbstract abstract = create_abstract(texture_node, file);
			graphics::Anim anim(abstract, scale, parse_spritesheet_options(texture_node));
			texture = load_anim(anim, texture_node, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = new CPUBuffer(graphics::AnimRef(std::move(anim)));
			if (params.abstract)
				*params.abstract = new graphics::NSVGAbstract(std::move(abstract));
		}
		else
		{
			graphics::NSVGAbstract abstract = create_abstract(texture_node, file);
			graphics::VectorImageRef image;
			image.scale = scale;
			image.image = context::nsvg_context().rasterize_res(abstract, scale);
			texture = load_svg(abstract, image, texture_node, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = new CPUBuffer(image);
			if (params.abstract)
				*params.abstract = new graphics::NSVGAbstract(std::move(abstract));
		}

		return texture;
	}
	
	void TextureRegistry::clear()
	{
		images.clear();
		anims.clear();
		vector_images.clear();
		textures.clear();
		svg_textures.clear();
		nsvg_abstracts.clear();
	}

	glm::vec2 TextureRegistry::get_dimensions(const std::string& file, unsigned int texture_index)
	{
		TextureKey tkey{ .file = file, .index = texture_index };
		{
			auto it = images.find(tkey);
			if (it != images.end())
				return it->second->dim().dimensions();
		}
		{
			auto it = anims.find(tkey);
			if (it != anims.end())
				return it->second->dimensions();
		}
		{
			auto it = vector_images.find(tkey);
			if (it != vector_images.end())
				return it->second.image->dim().dimensions();
		}
		throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	graphics::ImageDimensions TextureRegistry::get_image_dimensions(const std::string& file, unsigned int texture_index) const
	{
		TextureKey tkey{ .file = file, .index = texture_index };
		{
			auto it = images.find(tkey);
			if (it != images.end())
				return it->second->dim();
		}
		{
			auto it = vector_images.find(tkey);
			if (it != vector_images.end())
				return it->second.image->dim();
		}
		throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	std::weak_ptr<graphics::AnimDimensions> TextureRegistry::get_anim_dimensions(const std::string& file, unsigned int texture_index) const
	{
		auto it = anims.find({ .file = file, .index = texture_index });
		if (it != anims.end())
			return it->second->dim();
		else
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	graphics::ImageRef TextureRegistry::get_image_pixel_buffer(const std::string& file, unsigned int texture_index)
	{
		auto it = vector_images.find({ .file = file, .index = texture_index });
		if (it != vector_images.end())
			return it->second.image;
		else
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	graphics::AnimRef TextureRegistry::get_anim_pixel_buffer(const std::string& file, unsigned int texture_index)
	{
		auto it = anims.find({ .file = file, .index = texture_index });
		if (it != anims.end())
			return it->second;
		else
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	const graphics::NSVGAbstract& TextureRegistry::get_nsvg_abstract(const std::string& file, unsigned int texture_index) const
	{
		auto it = nsvg_abstracts.find({ .file = file, .index = texture_index });
		if (it != nsvg_abstracts.end())
			return it->second;
		throw Error(ErrorCode::UNREGISTERED_NSVG_ABSTRACT);
	}

	void TextureRegistry::free_texture(const std::string& file, unsigned int texture_index)
	{
		TextureKey tkey{ file, texture_index };
		{
			auto it = textures.find(tkey);
			if (it == textures.end())
			{
				free_svg_texture(file, 1.0f, texture_index);
				return;
			}
			textures.erase(it);
		}
		{
			auto it = images.find(tkey);
			if (it != images.end())
			{
				images.erase(it);
				return;
			}
		}
		{
			auto it = anims.find(tkey);
			if (it != anims.end())
			{
				anims.erase(it);
				return;
			}
		}
	}

	void TextureRegistry::free_svg_texture(const std::string& file, float scale, unsigned int texture_index)
	{
		{
			SVGTextureKey skey{ file, texture_index, scale };
			auto it = svg_textures.find(skey);
			if (it == svg_textures.end())
				return;
			svg_textures.erase(it);
		}
		TextureKey tkey{ file, texture_index };
		{
			auto it = vector_images.find(tkey);
			if (it != vector_images.end())
			{
				vector_images.erase(it);
				return;
			}
		}
		{
			auto it = anims.find(tkey);
			if (it != anims.end())
			{
				anims.erase(it);
				return;
			}
		}
	}

	void TextureRegistry::free_nsvg_abstract(const std::string& file, unsigned int texture_index)
	{
		TextureKey tkey{ file, texture_index };
		auto it = nsvg_abstracts.find(tkey);
		if (it != nsvg_abstracts.end())
			nsvg_abstracts.erase(it);
	}
}
