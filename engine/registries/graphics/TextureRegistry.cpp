#include "TextureRegistry.h"

#include "core/context/rendering/Textures.h"

#include "core/types/Meta.h"
#include "core/util/LoggerOperators.h"

#include "registries/Loader.h"

namespace oly::reg
{
	static void setup_texture(graphics::BindlessTexture& texture, TOMLNode node, bool set_and_use)
	{
		GLenum min_filter, mag_filter, wrap_s, wrap_t;
		if (!parse_min_filter(node["min_filter"], min_filter))
			min_filter = GL_NEAREST;
		texture.texture().set_parameter(GL_TEXTURE_MIN_FILTER, min_filter);
		if (!parse_mag_filter(node["mag_filter"], mag_filter))
			mag_filter = GL_NEAREST;
		texture.texture().set_parameter(GL_TEXTURE_MAG_FILTER, mag_filter);
		if (parse_wrap(node["wrap_s"], wrap_s))
			texture.texture().set_parameter(GL_TEXTURE_WRAP_S, wrap_s);
		if (parse_wrap(node["wrap_t"], wrap_t))
			texture.texture().set_parameter(GL_TEXTURE_WRAP_T, wrap_t);

		if (set_and_use)
			texture.set_and_use_handle();
	}

	static graphics::BindlessTextureRef load_image(const graphics::Image& image, TOMLNode node, bool set_and_use)
	{
		graphics::BindlessTexture texture = graphics::load_bindless_texture_2d(image, parse_bool_or(node["generate_mipmaps"], false));
		setup_texture(texture, node, set_and_use);
		return graphics::BindlessTextureRef(std::move(texture));
	}

	static graphics::BindlessTextureRef load_anim(const graphics::Anim& anim, TOMLNode node, bool set_and_use)
	{
		graphics::BindlessTexture texture = graphics::load_bindless_texture_2d_array(anim, parse_bool_or(node["generate_mipmaps"], false));
		setup_texture(texture, node, set_and_use);
		return graphics::BindlessTextureRef(std::move(texture));
	}

	static graphics::BindlessTextureRef load_svg(const graphics::NSVGAbstract& abstract, const graphics::VectorImageRef& image, TOMLNode node, bool set_and_use)
	{
		graphics::BindlessTextureRef texture;
		std::string generate_mipmaps = node["generate_mipmaps"].value<std::string>().value_or("off");
		graphics::SVGMipmapGenerationMode mipmaps_mode
			= generate_mipmaps == "auto" ? graphics::SVGMipmapGenerationMode::AUTO
			: generate_mipmaps == "manual" ? graphics::SVGMipmapGenerationMode::MANUAL
			: graphics::SVGMipmapGenerationMode::OFF;
		texture = graphics::BindlessTextureRef(graphics::load_bindless_nsvg_texture_2d(image, mipmaps_mode, mipmaps_mode == graphics::SVGMipmapGenerationMode::MANUAL ? &abstract : nullptr));
		setup_texture(*texture, node, set_and_use);
		return texture;
	}

	static void load_texture_node(const ResourcePath& file, toml::parse_result& toml, TOMLNode& texture_node, size_t texture_index)
	{
		toml = load_toml(file.get_import_path());
		auto texture_array = toml["texture"].as_array();
		if (texture_array && !texture_array->empty())
		{
			if (LOG.enable.debug)
			{
				auto src = toml["source"].value<std::string>();
				OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing texture [" << (src ? *src : "") << "]." << LOG.nl;
			}

			texture_index = glm::clamp(texture_index, size_t(0), texture_array->size() - size_t(1));
			texture_node = TOMLNode(*texture_array->get(texture_index));
		}
		else
			OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Missing or empty \"texture\" array field." << LOG.nl;
	}

	static bool should_store(TOMLNode texture_node, const char* storage_key, TextureRegistry::ImageStorageOverride storage_override)
	{
		if (storage_override == TextureRegistry::ImageStorageOverride::DISCARD)
			return false;
		else if (storage_override == TextureRegistry::ImageStorageOverride::KEEP)
			return true;
		else
			return texture_node[storage_key].value<std::string>().value_or("discard") == "keep";
	}

	static graphics::SpritesheetOptions parse_spritesheet_options(TOMLNode texture_node)
	{
		graphics::SpritesheetOptions options;
		parse_uint(texture_node["rows"], options.rows);
		parse_uint(texture_node["cols"], options.cols);
		parse_uint(texture_node["cell_width_override"], options.cell_width_override);
		parse_uint(texture_node["cell_height_override"], options.cell_height_override);
		parse_int(texture_node["delay_cs"], options.delay_cs);
		parse_bool(texture_node["row_major"], options.row_major);
		parse_bool(texture_node["row_up"], options.row_up);
		return options;
	}

	graphics::BindlessTextureRef TextureRegistry::load_texture(const ResourcePath& file, unsigned int texture_index, LoadParams params)
	{
		if (file.extension_matches(".svg"))
		{
			SVGLoadParams svg_params{
				.abstract_storage = ImageStorageOverride::DEFAULT,
				.image_storage = params.storage,
				.set_and_use = params.set_and_use
			};
			return load_svg_texture(file, texture_index, svg_params);
		}
		
		TextureKey key { file, texture_index };
		auto it = textures.find_forward_iterator(key);
		if (it != textures.forward_end())
			return it->second;

		toml::parse_result toml;
		TOMLNode texture_node;
		load_texture_node(file, toml, texture_node, texture_index);

		bool store_buffer = should_store(texture_node, "storage", params.storage);

		graphics::BindlessTextureRef texture;

		if (file.extension_matches(".gif"))
		{
			graphics::Anim anim(file);
			texture = load_anim(anim, texture_node, params.set_and_use);
			if (!store_buffer)
				anim.delete_buffer();
			anims[key] = graphics::AnimRef(std::move(anim));
		}
		else
		{
			if (parse_bool_or(texture_node["anim"], false))
			{
				graphics::Anim anim(file, parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (!store_buffer)
					anim.delete_buffer();
				anims[key] = graphics::AnimRef(std::move(anim));
			}
			else
			{
				graphics::Image image(file);
				texture = load_image(image, texture_node, params.set_and_use);
				if (!store_buffer)
					image.delete_buffer();
				images[key] = graphics::ImageRef(std::move(image));
			}
		}

		if (LOG.enable.debug)
		{
			auto src = toml["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Texture [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		textures.set(key, texture);
		return texture;
	}

	graphics::BindlessTextureRef TextureRegistry::load_svg_texture(const ResourcePath& file, unsigned int texture_index, SVGLoadParams params)
	{
		if (!file.extension_matches(".svg"))
			OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Attempting to load non-svg file as svg texture: " << file << LOG.nl;

		TextureKey key{ file, texture_index };
		auto it = textures.find_forward_iterator(key);
		if (it != textures.forward_end())
			return it->second;

		toml::parse_result toml;
		TOMLNode texture_node;
		load_texture_node(file, toml, texture_node, texture_index);

		bool store_abstract = should_store((TOMLNode)toml, "abstract_storage", params.abstract_storage);
		bool store_image = should_store(texture_node, "image_storage", params.image_storage);
		float scale = parse_float_or(texture_node["svg_scale"], 1.0f);

		graphics::BindlessTextureRef texture;

		if (parse_bool_or(texture_node["anim"], false))
		{
			auto ait = nsvg_abstracts.find(file);
			if (ait != nsvg_abstracts.end())
			{
				graphics::Anim anim(ait->second, scale, parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (!store_image)
					anim.delete_buffer();
				anims[key] = graphics::AnimRef(std::move(anim));
			}
			else
			{
				graphics::NSVGAbstract abstract(file);
				graphics::Anim anim(abstract, scale, parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (!store_image)
					anim.delete_buffer();
				anims[key] = graphics::AnimRef(std::move(anim));
				if (store_abstract)
					nsvg_abstracts.emplace(file, std::move(abstract));
			}
		}
		else
		{
			auto ait = nsvg_abstracts.find(file);
			if (ait != nsvg_abstracts.end())
			{
				graphics::VectorImageRef image;
				image.scale = scale;
				image.image = context::nsvg_context().rasterize_res(ait->second, scale);
				texture = load_svg(ait->second, image, texture_node, params.set_and_use);
				if (!store_image)
					image.image->delete_buffer();
				vector_images[key] = image;
			}
			else
			{
				graphics::NSVGAbstract abstract(file);
				graphics::VectorImageRef image;
				image.scale = scale;
				image.image = context::nsvg_context().rasterize_res(abstract, scale);
				texture = load_svg(abstract, image, texture_node, params.set_and_use);
				if (!store_image)
					image.image->delete_buffer();
				vector_images[key] = image;
				if (store_abstract)
					nsvg_abstracts.emplace(file, std::move(abstract));
			}
		}

		if (LOG.enable.debug)
		{
			auto src = toml["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Texture [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		textures.set(key, texture);
		return texture;
	}
	
	graphics::BindlessTextureRef TextureRegistry::load_temp_texture(const ResourcePath& file, unsigned int texture_index, TempLoadParams params) const
	{
		if (file.extension_matches(".svg"))
		{
			TempSVGLoadParams svg_params{
				.cpubuffer = params.cpubuffer,
				.abstract = nullptr,
				.set_and_use = params.set_and_use
			};
			return load_temp_svg_texture(file, texture_index, svg_params);
		}

		toml::parse_result toml;
		TOMLNode texture_node;
		load_texture_node(file, toml, texture_node, texture_index);

		graphics::BindlessTextureRef texture;

		std::string f = file.get_absolute().string();
		if (file.extension_matches(".gif"))
		{
			graphics::Anim anim(f.c_str());
			texture = load_anim(anim, texture_node, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = new CPUBuffer(graphics::AnimRef(std::move(anim)));
		}
		else
		{
			if (parse_bool_or(texture_node["anim"], false))
			{
				graphics::Anim anim(f.c_str(), parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (params.cpubuffer)
					*params.cpubuffer = new CPUBuffer(graphics::AnimRef(std::move(anim)));
			}
			else
			{
				graphics::Image image(f.c_str());
				texture = load_image(image, texture_node, params.set_and_use);
				if (params.cpubuffer)
					*params.cpubuffer = new CPUBuffer(graphics::ImageRef(std::move(image)));
			}
		}

		if (LOG.enable.debug)
		{
			auto src = toml["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Texture [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return texture;
	}
	
	graphics::BindlessTextureRef TextureRegistry::load_temp_svg_texture(const ResourcePath& file, unsigned int texture_index, TempSVGLoadParams params) const
	{
		if (!file.extension_matches(".svg"))
			OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Attempting to load non-svg file as svg texture: " << file << LOG.nl;

		toml::parse_result toml;
		TOMLNode texture_node;
		load_texture_node(file, toml, texture_node, texture_index);
		float scale = parse_float_or(texture_node["svg_scale"], 1.0f);

		graphics::BindlessTextureRef texture;

		if (parse_bool_or(texture_node["anim"], false))
		{
			graphics::NSVGAbstract abstract(file);
			graphics::Anim anim(abstract, scale, parse_spritesheet_options(texture_node));
			texture = load_anim(anim, texture_node, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = new CPUBuffer(graphics::AnimRef(std::move(anim)));
			if (params.abstract)
				*params.abstract = new graphics::NSVGAbstract(std::move(abstract));
		}
		else
		{
			graphics::NSVGAbstract abstract(file);
			graphics::VectorImageRef image;
			image.scale = scale;
			image.image = context::nsvg_context().rasterize_res(abstract, scale);
			texture = load_svg(abstract, image, texture_node, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = new CPUBuffer(image);
			if (params.abstract)
				*params.abstract = new graphics::NSVGAbstract(std::move(abstract));
		}

		if (LOG.enable.debug)
		{
			auto src = toml["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Texture [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return texture;
	}
	
	void TextureRegistry::clear()
	{
		images.clear();
		anims.clear();
		vector_images.clear();
		textures.clear();
		nsvg_abstracts.clear();
	}

	glm::vec2 TextureRegistry::get_dimensions(const ResourcePath& file, unsigned int texture_index) const
	{
		return get_dimensions({ .file = file, .index = texture_index });
	}

	graphics::ImageDimensions TextureRegistry::get_image_dimensions(const ResourcePath& file, unsigned int texture_index) const
	{
		return get_image_dimensions({ .file = file, .index = texture_index });
	}

	std::weak_ptr<graphics::AnimDimensions> TextureRegistry::get_anim_dimensions(const ResourcePath& file, unsigned int texture_index) const
	{
		return get_anim_dimensions({ .file = file, .index = texture_index });
	}

	graphics::ImageRef TextureRegistry::get_image_pixel_buffer(const ResourcePath& file, unsigned int texture_index)
	{
		return get_image_pixel_buffer({ .file = file, .index = texture_index });
	}

	graphics::AnimRef TextureRegistry::get_anim_pixel_buffer(const ResourcePath& file, unsigned int texture_index)
	{
		return get_anim_pixel_buffer({ .file = file, .index = texture_index });
	}

	glm::vec2 TextureRegistry::get_dimensions(const graphics::BindlessTextureRef& texture) const
	{
		auto it = textures.find_backward_iterator(texture);
		if (it == textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_dimensions(it->second);
	}

	graphics::ImageDimensions TextureRegistry::get_image_dimensions(const graphics::BindlessTextureRef& texture) const
	{
		auto it = textures.find_backward_iterator(texture);
		if (it == textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_image_dimensions(it->second);
	}

	std::weak_ptr<graphics::AnimDimensions> TextureRegistry::get_anim_dimensions(const graphics::BindlessTextureRef& texture) const
	{
		auto it = textures.find_backward_iterator(texture);
		if (it == textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_anim_dimensions(it->second);
	}

	graphics::ImageRef TextureRegistry::get_image_pixel_buffer(const graphics::BindlessTextureRef& texture)
	{
		auto it = textures.find_backward_iterator(texture);
		if (it == textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_image_pixel_buffer(it->second);
	}

	graphics::AnimRef TextureRegistry::get_anim_pixel_buffer(const graphics::BindlessTextureRef& texture)
	{
		auto it = textures.find_backward_iterator(texture);
		if (it == textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_anim_pixel_buffer(it->second);
	}

	glm::vec2 TextureRegistry::get_dimensions(const TextureKey& key) const
	{
		{
			auto it = images.find(key);
			if (it != images.end())
				return it->second->dim().dimensions();
		}

		{
			auto it = anims.find(key);
			if (it != anims.end())
				return it->second->dimensions();
		}
		
		{
			auto it = vector_images.find(key);
			if (it != vector_images.end())
				return it->second.image->dim().dimensions();
		}
		
		throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	graphics::ImageDimensions TextureRegistry::get_image_dimensions(const TextureKey& key) const
	{
		{
			auto it = images.find(key);
			if (it != images.end())
				return it->second->dim();
		}

		{
			auto it = vector_images.find(key);
			if (it != vector_images.end())
				return it->second.image->dim();
		}
		
		throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	std::weak_ptr<graphics::AnimDimensions> TextureRegistry::get_anim_dimensions(const TextureKey& key) const
	{
		auto it = anims.find(key);
		if (it != anims.end())
			return it->second->dim();
		else
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	graphics::ImageRef TextureRegistry::get_image_pixel_buffer(const TextureKey& key)
	{
		auto it = vector_images.find(key);
		if (it != vector_images.end())
			return it->second.image;
		else
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	graphics::AnimRef TextureRegistry::get_anim_pixel_buffer(const TextureKey& key)
	{
		auto it = anims.find(key);
		if (it != anims.end())
			return it->second;
		else
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	const graphics::NSVGAbstract& TextureRegistry::get_nsvg_abstract(const ResourcePath& file) const
	{
		auto it = nsvg_abstracts.find(file);
		if (it != nsvg_abstracts.end())
			return it->second;
		throw Error(ErrorCode::UNREGISTERED_NSVG_ABSTRACT);
	}

	void TextureRegistry::free_texture(const ResourcePath& file, unsigned int texture_index)
	{
		TextureKey key{ file, texture_index };

		{
			auto it = textures.find_forward_iterator(key);
			if (it == textures.forward_end())
			{
				free_svg_texture(file, texture_index);
				return;
			}
			textures.forward_erase(it);
		}

		{
			auto it = images.find(key);
			if (it != images.end())
			{
				images.erase(it);
				return;
			}
		}

		{
			auto it = anims.find(key);
			if (it != anims.end())
			{
				anims.erase(it);
				return;
			}
		}
	}

	void TextureRegistry::free_svg_texture(const ResourcePath& file, unsigned int texture_index)
	{
		TextureKey key{ file, texture_index };

		{
			auto it = textures.find_forward_iterator(key);
			if (it == textures.forward_end())
				return;
			textures.forward_erase(it);
		}

		{
			auto it = vector_images.find(key);
			if (it != vector_images.end())
			{
				vector_images.erase(it);
				return;
			}
		}

		{
			auto it = anims.find(key);
			if (it != anims.end())
			{
				anims.erase(it);
				return;
			}
		}
	}

	void TextureRegistry::free_nsvg_abstract(const ResourcePath& file)
	{
		auto it = nsvg_abstracts.find(file);
		if (it != nsvg_abstracts.end())
			nsvg_abstracts.erase(it);
	}
}
