#include "Textures.h"

#include "core/context/rendering/Sprites.h"
#include "registries/graphics/sprites/Sprites.h"

#include "core/types/Meta.h"
#include "core/util/LoggerOperators.h"
#include "registries/Loader.h"
#include "registries/MetaSplitter.h"

#include "core/containers/Bijection.h"

namespace oly::context
{
	namespace internal
	{
		graphics::NSVGContext nsvg_context;

		struct TextureKey
		{
			ResourcePath file;
			unsigned int index;

			bool operator==(const TextureKey&) const = default;
		};

		struct TextureHash
		{
			size_t operator()(const TextureKey& k) const { return std::hash<ResourcePath>{}(k.file) ^ std::hash<unsigned int>{}(k.index); }
		};

		std::unordered_map<TextureKey, graphics::ImageRef, TextureHash> images;
		std::unordered_map<TextureKey, graphics::AnimRef, TextureHash> anims;
		std::unordered_map<TextureKey, graphics::VectorImageRef, TextureHash> vector_images;

		std::unordered_map<ResourcePath, graphics::NSVGAbstract> nsvg_abstracts;

		Bijection<TextureKey, graphics::BindlessTextureRef, TextureHash> textures;
	}

	void internal::terminate_textures()
	{
		internal::images.clear();
		internal::anims.clear();
		internal::vector_images.clear();
		internal::textures.clear();
		internal::nsvg_abstracts.clear();
	}

	graphics::NSVGContext& nsvg_context()
	{
		return internal::nsvg_context;
	}

	void sync_texture_handle(const graphics::BindlessTextureRef& texture)
	{
		rendering::internal::SpriteBatchRegistry::instance().update_texture_handle(texture);
	}

	static void setup_texture(graphics::BindlessTexture& texture, TOMLNode node, bool set_and_use)
	{
		GLenum min_filter, mag_filter, wrap_s, wrap_t;
		if (!reg::parse_min_filter(node["min_filter"], min_filter))
			min_filter = GL_NEAREST;
		texture.texture().set_parameter(GL_TEXTURE_MIN_FILTER, min_filter);
		if (!reg::parse_mag_filter(node["mag_filter"], mag_filter))
			mag_filter = GL_NEAREST;
		texture.texture().set_parameter(GL_TEXTURE_MAG_FILTER, mag_filter);
		if (reg::parse_wrap(node["wrap_s"], wrap_s))
			texture.texture().set_parameter(GL_TEXTURE_WRAP_S, wrap_s);
		if (reg::parse_wrap(node["wrap_t"], wrap_t))
			texture.texture().set_parameter(GL_TEXTURE_WRAP_T, wrap_t);

		if (set_and_use)
			texture.set_and_use_handle();
	}

	static graphics::BindlessTextureRef load_image(const graphics::Image& image, TOMLNode node, bool set_and_use)
	{
		graphics::BindlessTexture texture = graphics::load_bindless_texture_2d(image, reg::parse_bool_or(node["generate_mipmaps"], false));
		setup_texture(texture, node, set_and_use);
		return graphics::BindlessTextureRef(std::move(texture));
	}

	static graphics::BindlessTextureRef load_anim(const graphics::Anim& anim, TOMLNode node, bool set_and_use)
	{
		graphics::BindlessTexture texture = graphics::load_bindless_texture_2d_array(anim, reg::parse_bool_or(node["generate_mipmaps"], false));
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

	static TOMLNode load_texture_node(const ResourcePath& file, toml::parse_result& toml, unsigned int texture_index)
	{
		OLY_LOG_DEBUG(true, "CONTEXT") << LOG.source_info.full_source() << "Parsing texture [" << file << "]..." << LOG.nl;

		ResourcePath import_file = file.get_import_path();
		if (!reg::MetaSplitter::meta(import_file).has_type("texture"))
		{
			OLY_LOG_ERROR(true, "CONTEXT") << LOG.source_info.full_source() << "Meta fields do not contain texture type." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		toml = reg::load_toml(import_file);
		auto texture_array = toml["texture"].as_array();
		if (!texture_array)
		{
			OLY_LOG_ERROR(true, "CONTEXT") << LOG.source_info.full_source() << "Missing \"texture\" array field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (texture_index >= texture_array->size())
		{
			OLY_LOG_ERROR(true, "CONTEXT") << LOG.source_info.full_source() << "Texture index (" << texture_index
				<< ") out of range for texture array size (" << texture_array->size() << ")." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		return TOMLNode(*texture_array->get(texture_index));
	}

	static bool should_store(TOMLNode texture_node, const char* storage_key, tex::ImageStorageOverride storage_override)
	{
		if (storage_override == tex::ImageStorageOverride::DISCARD)
			return false;
		else if (storage_override == tex::ImageStorageOverride::KEEP)
			return true;
		else
			return texture_node[storage_key].value<std::string>().value_or("discard") == "keep";
	}

	static graphics::SpritesheetOptions parse_spritesheet_options(TOMLNode texture_node)
	{
		graphics::SpritesheetOptions options;
		reg::parse_uint(texture_node["rows"], options.rows);
		reg::parse_uint(texture_node["cols"], options.cols);
		reg::parse_uint(texture_node["cell_width_override"], options.cell_width_override);
		reg::parse_uint(texture_node["cell_height_override"], options.cell_height_override);
		reg::parse_int(texture_node["delay_cs"], options.delay_cs);
		reg::parse_bool(texture_node["row_major"], options.row_major);
		reg::parse_bool(texture_node["row_up"], options.row_up);
		return options;
	}

	graphics::BindlessTextureRef load_texture(const ResourcePath& file, unsigned int texture_index, tex::LoadParams params)
	{
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "CONTEXT") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (file.extension_matches(".svg"))
		{
			tex::SVGLoadParams svg_params{
				.abstract_storage = tex::ImageStorageOverride::DEFAULT,
				.image_storage = params.storage,
				.set_and_use = params.set_and_use
			};
			return load_svg_texture(file, texture_index, svg_params);
		}

		internal::TextureKey key{ file, texture_index };
		auto it = internal::textures.find_forward_iterator(key);
		if (it != internal::textures.forward_end())
			return it->second;

		toml::parse_result toml;
		TOMLNode texture_node = load_texture_node(file, toml, texture_index);

		bool store_buffer = should_store(texture_node, "storage", params.storage);

		graphics::BindlessTextureRef texture;

		if (file.extension_matches(".gif"))
		{
			graphics::Anim anim(file);
			texture = load_anim(anim, texture_node, params.set_and_use);
			if (!store_buffer)
				anim.delete_buffer();
			internal::anims[key] = graphics::AnimRef(std::move(anim));
		}
		else
		{
			if (reg::parse_bool_or(texture_node["anim"], false))
			{
				graphics::Anim anim(file, parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (!store_buffer)
					anim.delete_buffer();
				internal::anims[key] = graphics::AnimRef(std::move(anim));
			}
			else
			{
				graphics::Image image(file);
				texture = load_image(image, texture_node, params.set_and_use);
				if (!store_buffer)
					image.delete_buffer();
				internal::images[key] = graphics::ImageRef(std::move(image));
			}
		}

		OLY_LOG_DEBUG(true, "CONTEXT") << LOG.source_info.full_source() << "...Texture [" << file << "] parsed." << LOG.nl;

		internal::textures.set(key, texture);
		return texture;
	}

	graphics::BindlessTextureRef load_svg_texture(const ResourcePath& file, unsigned int texture_index, tex::SVGLoadParams params)
	{
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "CONTEXT") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (!file.extension_matches(".svg"))
			OLY_LOG_WARNING(true, "CONTEXT") << LOG.source_info.full_source() << "Attempting to load non-svg file as svg texture: " << file << LOG.nl;

		internal::TextureKey key{ file, texture_index };
		auto it = internal::textures.find_forward_iterator(key);
		if (it != internal::textures.forward_end())
			return it->second;

		toml::parse_result toml;
		TOMLNode texture_node = load_texture_node(file, toml, texture_index);

		bool store_abstract = should_store((TOMLNode)toml, "abstract_storage", params.abstract_storage);
		bool store_image = should_store(texture_node, "image_storage", params.image_storage);
		float scale = reg::parse_float_or(texture_node["svg_scale"], 1.0f);

		graphics::BindlessTextureRef texture;

		if (reg::parse_bool_or(texture_node["anim"], false))
		{
			auto ait = internal::nsvg_abstracts.find(file);
			if (ait != internal::nsvg_abstracts.end())
			{
				graphics::Anim anim(ait->second, scale, parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (!store_image)
					anim.delete_buffer();
				internal::anims[key] = graphics::AnimRef(std::move(anim));
			}
			else
			{
				graphics::NSVGAbstract abstract(file);
				graphics::Anim anim(abstract, scale, parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (!store_image)
					anim.delete_buffer();
				internal::anims[key] = graphics::AnimRef(std::move(anim));
				if (store_abstract)
					internal::nsvg_abstracts.emplace(file, std::move(abstract));
			}
		}
		else
		{
			auto ait = internal::nsvg_abstracts.find(file);
			if (ait != internal::nsvg_abstracts.end())
			{
				graphics::VectorImageRef image;
				image.scale = scale;
				image.image = context::nsvg_context().rasterize_res(ait->second, scale);
				texture = load_svg(ait->second, image, texture_node, params.set_and_use);
				if (!store_image)
					image.image->delete_buffer();
				internal::vector_images[key] = image;
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
				internal::vector_images[key] = image;
				if (store_abstract)
					internal::nsvg_abstracts.emplace(file, std::move(abstract));
			}
		}

		OLY_LOG_DEBUG(true, "CONTEXT") << LOG.source_info.full_source() << "...Texture [" << file << "] parsed." << LOG.nl;

		internal::textures.set(key, texture);
		return texture;
	}

	graphics::BindlessTextureRef load_temp_texture(const ResourcePath& file, unsigned int texture_index, tex::TempLoadParams params)
	{
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "CONTEXT") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (file.extension_matches(".svg"))
		{
			tex::TempSVGLoadParams svg_params{
				.cpubuffer = params.cpubuffer,
				.abstract = nullptr,
				.set_and_use = params.set_and_use
			};
			return load_temp_svg_texture(file, texture_index, svg_params);
		}

		toml::parse_result toml;
		TOMLNode texture_node = load_texture_node(file, toml, texture_index);

		graphics::BindlessTextureRef texture;

		std::string f = file.get_absolute().string();
		if (file.extension_matches(".gif"))
		{
			graphics::Anim anim(f.c_str());
			texture = load_anim(anim, texture_node, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = graphics::AnimRef(std::move(anim));
		}
		else
		{
			if (reg::parse_bool_or(texture_node["anim"], false))
			{
				graphics::Anim anim(f.c_str(), parse_spritesheet_options(texture_node));
				texture = load_anim(anim, texture_node, params.set_and_use);
				if (params.cpubuffer)
					*params.cpubuffer = tex::CPUBuffer(graphics::AnimRef(std::move(anim)));
			}
			else
			{
				graphics::Image image(f.c_str());
				texture = load_image(image, texture_node, params.set_and_use);
				if (params.cpubuffer)
					*params.cpubuffer = tex::CPUBuffer(graphics::ImageRef(std::move(image)));
			}
		}

		OLY_LOG_DEBUG(true, "CONTEXT") << LOG.source_info.full_source() << "...Texture [" << file << "] parsed." << LOG.nl;

		return texture;
	}

	graphics::BindlessTextureRef load_temp_svg_texture(const ResourcePath& file, unsigned int texture_index, tex::TempSVGLoadParams params)
	{
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "CONTEXT") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (!file.extension_matches(".svg"))
			OLY_LOG_WARNING(true, "CONTEXT") << LOG.source_info.full_source() << "Attempting to load non-svg file as svg texture: " << file << LOG.nl;

		toml::parse_result toml;
		TOMLNode texture_node = load_texture_node(file, toml, texture_index);
		float scale = reg::parse_float_or(texture_node["svg_scale"], 1.0f);

		graphics::BindlessTextureRef texture;

		if (reg::parse_bool_or(texture_node["anim"], false))
		{
			graphics::NSVGAbstract abstract(file);
			graphics::Anim anim(abstract, scale, parse_spritesheet_options(texture_node));
			texture = load_anim(anim, texture_node, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = tex::CPUBuffer(graphics::AnimRef(std::move(anim)));
			if (params.abstract)
				params.abstract->init(graphics::NSVGAbstract(std::move(abstract)));
		}
		else
		{
			graphics::NSVGAbstract abstract(file);
			graphics::VectorImageRef image;
			image.scale = scale;
			image.image = context::nsvg_context().rasterize_res(abstract, scale);
			texture = load_svg(abstract, image, texture_node, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = tex::CPUBuffer(image);
			if (params.abstract)
				params.abstract->init(graphics::NSVGAbstract(std::move(abstract)));
		}

		OLY_LOG_DEBUG(true, "CONTEXT") << LOG.source_info.full_source() << "...Texture [" << file << "] parsed." << LOG.nl;

		return texture;
	}

	static glm::vec2 get_texture_dimensions(const internal::TextureKey& key)
	{
		{
			auto it = internal::images.find(key);
			if (it != internal::images.end())
				return it->second->dim().dimensions();
		}

		{
			auto it = internal::anims.find(key);
			if (it != internal::anims.end())
				return it->second->dimensions();
		}

		{
			auto it = internal::vector_images.find(key);
			if (it != internal::vector_images.end())
				return it->second.image->dim().dimensions();
		}

		throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	static graphics::ImageDimensions get_image_dimensions(const internal::TextureKey& key)
	{
		{
			auto it = internal::images.find(key);
			if (it != internal::images.end())
				return it->second->dim();
		}

		{
			auto it = internal::vector_images.find(key);
			if (it != internal::vector_images.end())
				return it->second.image->dim();
		}

		throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	static SmartReference<graphics::AnimDimensions> get_anim_dimensions(const internal::TextureKey& key)
	{
		auto it = internal::anims.find(key);
		if (it != internal::anims.end())
			return it->second->dim();
		else
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	static graphics::ImageRef get_image_pixel_buffer(const internal::TextureKey& key)
	{
		auto it = internal::vector_images.find(key);
		if (it != internal::vector_images.end())
			return it->second.image;
		else
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	static graphics::AnimRef get_anim_pixel_buffer(const internal::TextureKey& key)
	{
		auto it = internal::anims.find(key);
		if (it != internal::anims.end())
			return it->second;
		else
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
	}

	glm::vec2 get_texture_dimensions(const ResourcePath& file, unsigned int texture_index)
	{
		return get_texture_dimensions({ .file = file, .index = texture_index });
	}

	graphics::ImageDimensions get_image_dimensions(const ResourcePath& file, unsigned int texture_index)
	{
		return get_image_dimensions({ .file = file, .index = texture_index });
	}

	SmartReference<graphics::AnimDimensions> get_anim_dimensions(const ResourcePath& file, unsigned int texture_index)
	{
		return get_anim_dimensions({ .file = file, .index = texture_index });
	}

	graphics::ImageRef get_image_pixel_buffer(const ResourcePath& file, unsigned int texture_index)
	{
		return get_image_pixel_buffer({ .file = file, .index = texture_index });
	}

	graphics::AnimRef get_anim_pixel_buffer(const ResourcePath& file, unsigned int texture_index)
	{
		return get_anim_pixel_buffer({ .file = file, .index = texture_index });
	}

	glm::vec2 get_texture_dimensions(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_texture_dimensions(it->second);
	}

	graphics::ImageDimensions get_image_dimensions(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_image_dimensions(it->second);
	}

	SmartReference<graphics::AnimDimensions> get_anim_dimensions(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_anim_dimensions(it->second);
	}

	graphics::ImageRef get_image_pixel_buffer(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_image_pixel_buffer(it->second);
	}

	graphics::AnimRef get_anim_pixel_buffer(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UNREGISTERED_TEXTURE);
		return get_anim_pixel_buffer(it->second);
	}

	const graphics::NSVGAbstract& get_nsvg_abstract(const ResourcePath& file)
	{
		auto it = internal::nsvg_abstracts.find(file);
		if (it != internal::nsvg_abstracts.end())
			return it->second;
		throw Error(ErrorCode::UNREGISTERED_NSVG_ABSTRACT);
	}

	void free_texture(const ResourcePath& file, unsigned int texture_index)
	{
		internal::TextureKey key{ file, texture_index };

		{
			auto it = internal::textures.find_forward_iterator(key);
			if (it == internal::textures.forward_end())
			{
				free_svg_texture(file, texture_index);
				return;
			}
			internal::textures.forward_erase(it);
		}

		{
			auto it = internal::images.find(key);
			if (it != internal::images.end())
			{
				internal::images.erase(it);
				return;
			}
		}

		{
			auto it = internal::anims.find(key);
			if (it != internal::anims.end())
			{
				internal::anims.erase(it);
				return;
			}
		}
	}

	void free_svg_texture(const ResourcePath& file, unsigned int texture_index)
	{
		internal::TextureKey key{ file, texture_index };

		{
			auto it = internal::textures.find_forward_iterator(key);
			if (it == internal::textures.forward_end())
				return;
			internal::textures.forward_erase(it);
		}

		{
			auto it = internal::vector_images.find(key);
			if (it != internal::vector_images.end())
			{
				internal::vector_images.erase(it);
				return;
			}
		}

		{
			auto it = internal::anims.find(key);
			if (it != internal::anims.end())
			{
				internal::anims.erase(it);
				return;
			}
		}
	}

	void free_nsvg_abstract(const ResourcePath& file)
	{
		auto it = internal::nsvg_abstracts.find(file);
		if (it != internal::nsvg_abstracts.end())
			internal::nsvg_abstracts.erase(it);
	}
}
