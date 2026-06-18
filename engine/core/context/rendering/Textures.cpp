#include "Textures.h"

#include "core/context/rendering/Sprites.h"
#include "core/containers/Bijection.h"
#include "core/types/Meta.h"
#include "core/util/LoggerOperators.h"
#include "core/util/Loader.h"
#include "core/util/Parser.h"

#include "assets/MetaSplitter.h"
#include "definitions/Keys.h"
#include "definitions/enums/StorageMode.h"
#include "definitions/enums/SVGMipmapGenerationMode.h"

namespace oly::context
{
	namespace internal
	{
		graphics::NSVGContext nsvg_context;

		struct TextureKey
		{
			detail::ResourcePath file;
			unsigned int index;

			bool operator==(const TextureKey&) const = default;
		};

		struct TextureHash
		{
			size_t operator()(const TextureKey& k) const { return k.file.hash() ^ std::hash<unsigned int>{}(k.index); }
		};

		std::unordered_map<TextureKey, graphics::ImageRef, TextureHash> images;
		std::unordered_map<TextureKey, graphics::AnimRef, TextureHash> anims;
		std::unordered_map<TextureKey, graphics::VectorImageRef, TextureHash> vector_images;

		std::unordered_map<detail::ResourcePath, graphics::NSVGAbstract> nsvg_abstracts;

		Bijection<TextureKey, graphics::BindlessTextureRef, TextureHash> textures;
	}

	struct TexturesOnTerminate
	{
		void operator()()
		{
			internal::images.clear();
			internal::anims.clear();
			internal::vector_images.clear();
			internal::textures.clear();
			internal::nsvg_abstracts.clear();
		}
	};

	void internal::init_textures()
	{
		SingletonTickService<TickPhase::None, void, TerminatePhase::Graphics, TexturesOnTerminate>::instance();
	}

	graphics::NSVGContext& nsvg_context()
	{
		return internal::nsvg_context;
	}

	void sync_texture_handle(const graphics::BindlessTextureRef& texture)
	{
		rendering::internal::update_texture_handle(texture);
	}

	static void setup_texture(graphics::BindlessTexture& texture, const assets::Parser& parser, bool set_and_use)
	{
		texture.texture().set_parameter(GL_TEXTURE_MIN_FILTER, parser.required<GLenum>(detail::Key::MinFilter)());
		texture.texture().set_parameter(GL_TEXTURE_MAG_FILTER, parser.required<GLenum>(detail::Key::MagFilter)());
		texture.texture().set_parameter(GL_TEXTURE_WRAP_S, parser.defaulted(detail::Key::WrapS)(GL_CLAMP_TO_EDGE));
		texture.texture().set_parameter(GL_TEXTURE_WRAP_T, parser.defaulted(detail::Key::WrapT)(GL_CLAMP_TO_EDGE));

		if (set_and_use)
			texture.set_and_use_handle();
	}

	static graphics::BindlessTextureRef load_image(const graphics::Image& image, const assets::Parser& parser, bool set_and_use)
	{
		graphics::BindlessTexture texture = graphics::load_bindless_texture_2d(image, parser.defaulted(detail::Key::GenerateMipmaps)(false));
		setup_texture(texture, parser, set_and_use);
		return graphics::BindlessTextureRef(std::move(texture));
	}

	static graphics::BindlessTextureRef load_anim(const graphics::Anim& anim, const assets::Parser& parser, bool set_and_use)
	{
		graphics::BindlessTexture texture = graphics::load_bindless_texture_2d_array(anim, parser.defaulted(detail::Key::GenerateMipmaps)(false));
		setup_texture(texture, parser, set_and_use);
		return graphics::BindlessTextureRef(std::move(texture));
	}

	static graphics::BindlessTextureRef load_svg(const graphics::NSVGAbstract& abstract, const graphics::VectorImageRef& image, const assets::Parser& parser, bool set_and_use)
	{
		graphics::BindlessTextureRef texture;
		auto mipmaps_mode = parser.defaulted(detail::Key::GenerateMipmaps)(detail::SVGMipmapGenerationMode::Off);
		texture = graphics::BindlessTextureRef(graphics::load_bindless_nsvg_texture_2d(image, mipmaps_mode, mipmaps_mode == detail::SVGMipmapGenerationMode::Manual ? &abstract : nullptr));
		setup_texture(*texture, parser, set_and_use);
		return texture;
	}

	static assets::Parser load_texture_node(const detail::ResourcePath& file, toml::parse_result& toml, unsigned int texture_index)
	{
		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "Parsing texture [" << file << "]..." << LOG.nl;

		detail::ResourcePath import_file = file.get_import_path();
		if (!detail::MetaSplitter::decode_meta(import_file).has_type(detail::Key::Meta_Texture))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain texture type." << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		toml = io::load_toml(import_file);
		const auto texture_array = assets::Parser(toml).required<TOMLArray>(detail::Key::TextureArray)();

		if (texture_index >= texture_array->size())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Texture index (" << texture_index
				<< ") out of range for texture array size (" << texture_array->size() << ")" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		return assets::Parser((TOMLNode)*texture_array->get(texture_index));
	}

	static bool should_store(const assets::Parser& parser, detail::Key storage_key, tex::ImageStorageOverride storage_override, detail::StorageMode default_storage)
	{
		if (storage_override == tex::ImageStorageOverride::Discard)
			return false;
		else if (storage_override == tex::ImageStorageOverride::Keep)
			return true;
		else
			return parser.defaulted(storage_key)(default_storage) == detail::StorageMode::Keep;
	}

	static graphics::SpritesheetOptions parse_spritesheet_options(const assets::Parser& parser)
	{
		graphics::SpritesheetOptions options;
		parser.optional(detail::Key::RowType)(options.row_type);
		parser.optional(detail::Key::RowValue)(options.row_value);
		parser.optional(detail::Key::ColType)(options.col_type);
		parser.optional(detail::Key::ColValue)(options.col_value);
		parser.optional(detail::Key::RowOffsetIndex)(options.row_offset_index);
		parser.optional(detail::Key::RowOffsetPixel)(options.row_offset_pixel);
		parser.optional(detail::Key::ColOffsetIndex)(options.col_offset_index);
		parser.optional(detail::Key::ColOffsetPixel)(options.col_offset_pixel);
		parser.optional(detail::Key::Delay)(options.delay);
		parser.optional(detail::Key::RowMajor)(options.row_major);
		parser.optional(detail::Key::RowUp)(options.row_up);
		return options;
	}

	graphics::BindlessTextureRef load_texture(const detail::ResourcePath& file, unsigned int texture_index, tex::LoadParams params)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		if (file.extension_matches(".svg"))
		{
			tex::SVGLoadParams svg_params{
				.abstract_storage = tex::ImageStorageOverride::Default,
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
		assets::Parser parser = load_texture_node(file, toml, texture_index);

		bool store_buffer = should_store(parser, detail::Key::Storage, params.storage, detail::StorageMode::Keep);

		graphics::BindlessTextureRef texture;

		if (file.extension_matches(".gif"))
		{
			graphics::Anim anim(file);
			texture = load_anim(anim, parser, params.set_and_use);
			if (!store_buffer)
				anim.delete_buffer();
			internal::anims[key] = graphics::AnimRef(std::move(anim));
		}
		else
		{
			if (parser.defaulted(detail::Key::Animated)(false))
			{
				graphics::Anim anim(file, parse_spritesheet_options(parser));
				texture = load_anim(anim, parser, params.set_and_use);
				if (!store_buffer)
					anim.delete_buffer();
				internal::anims[key] = graphics::AnimRef(std::move(anim));
			}
			else
			{
				graphics::Image image(file);
				texture = load_image(image, parser, params.set_and_use);
				if (!store_buffer)
					image.delete_buffer();
				internal::images[key] = graphics::ImageRef(std::move(image));
			}
		}

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Texture [" << file << "] parsed" << LOG.nl;

		internal::textures.set(key, texture);
		return texture;
	}

	graphics::BindlessTextureRef load_svg_texture(const detail::ResourcePath& file, unsigned int texture_index, tex::SVGLoadParams params)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		if (!file.extension_matches(".svg"))
			_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Attempting to load non-svg file as svg texture: " << file << LOG.nl;

		internal::TextureKey key{ file, texture_index };
		auto it = internal::textures.find_forward_iterator(key);
		if (it != internal::textures.forward_end())
			return it->second;

		toml::parse_result toml;
		assets::Parser parser = load_texture_node(file, toml, texture_index);

		bool store_abstract = should_store(parser, detail::Key::AbstractStorage, params.abstract_storage, detail::StorageMode::Discard);
		bool store_image = should_store(parser, detail::Key::ImageStorage, params.image_storage, detail::StorageMode::Keep);
		float scale = parser.defaulted(detail::Key::VectorScale)(1.f);

		graphics::BindlessTextureRef texture;

		if (parser.defaulted(detail::Key::Animated)(false))
		{
			auto ait = internal::nsvg_abstracts.find(file);
			if (ait != internal::nsvg_abstracts.end())
			{
				graphics::Anim anim(ait->second, scale, parse_spritesheet_options(parser));
				texture = load_anim(anim, parser, params.set_and_use);
				if (!store_image)
					anim.delete_buffer();
				internal::anims[key] = graphics::AnimRef(std::move(anim));
			}
			else
			{
				graphics::NSVGAbstract abstract(file);
				graphics::Anim anim(abstract, scale, parse_spritesheet_options(parser));
				texture = load_anim(anim, parser, params.set_and_use);
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
				texture = load_svg(ait->second, image, parser, params.set_and_use);
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
				texture = load_svg(abstract, image, parser, params.set_and_use);
				if (!store_image)
					image.image->delete_buffer();
				internal::vector_images[key] = image;
				if (store_abstract)
					internal::nsvg_abstracts.emplace(file, std::move(abstract));
			}
		}

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Texture [" << file << "] parsed" << LOG.nl;

		internal::textures.set(key, texture);
		return texture;
	}

	graphics::BindlessTextureRef load_temp_texture(const detail::ResourcePath& file, unsigned int texture_index, tex::TempLoadParams params)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
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
		assets::Parser parser = load_texture_node(file, toml, texture_index);

		graphics::BindlessTextureRef texture;

		std::string f = file.get_absolute().string();
		if (file.extension_matches(".gif"))
		{
			graphics::Anim anim(f.c_str());
			texture = load_anim(anim, parser, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = graphics::AnimRef(std::move(anim));
		}
		else
		{
			if (parser.defaulted(detail::Key::Animated)(false))
			{
				graphics::Anim anim(f.c_str(), parse_spritesheet_options(parser));
				texture = load_anim(anim, parser, params.set_and_use);
				if (params.cpubuffer)
					*params.cpubuffer = tex::CPUBuffer(graphics::AnimRef(std::move(anim)));
			}
			else
			{
				graphics::Image image(f.c_str());
				texture = load_image(image, parser, params.set_and_use);
				if (params.cpubuffer)
					*params.cpubuffer = tex::CPUBuffer(graphics::ImageRef(std::move(image)));
			}
		}

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Texture [" << file << "] parsed" << LOG.nl;

		return texture;
	}

	graphics::BindlessTextureRef load_temp_svg_texture(const detail::ResourcePath& file, unsigned int texture_index, tex::TempSVGLoadParams params)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		if (!file.extension_matches(".svg"))
			_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Attempting to load non-svg file as svg texture: " << file << LOG.nl;

		toml::parse_result toml;
		assets::Parser parser = load_texture_node(file, toml, texture_index);
		float scale = parser.defaulted(detail::Key::VectorScale)(1.0f);

		graphics::BindlessTextureRef texture;

		if (parser.defaulted(detail::Key::Animated)(false))
		{
			graphics::NSVGAbstract abstract(file);
			graphics::Anim anim(abstract, scale, parse_spritesheet_options(parser));
			texture = load_anim(anim, parser, params.set_and_use);
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
			texture = load_svg(abstract, image, parser, params.set_and_use);
			if (params.cpubuffer)
				*params.cpubuffer = tex::CPUBuffer(image);
			if (params.abstract)
				params.abstract->init(graphics::NSVGAbstract(std::move(abstract)));
		}

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Texture [" << file << "] parsed" << LOG.nl;

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

		throw Error(ErrorCode::UnregisteredTexture);
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

		throw Error(ErrorCode::UnregisteredTexture);
	}

	static SmartReference<graphics::AnimDimensions> get_anim_dimensions(const internal::TextureKey& key)
	{
		auto it = internal::anims.find(key);
		if (it != internal::anims.end())
			return it->second->dim();
		else
			throw Error(ErrorCode::UnregisteredTexture);
	}

	static graphics::ImageRef get_image_pixel_buffer(const internal::TextureKey& key)
	{
		auto it = internal::vector_images.find(key);
		if (it != internal::vector_images.end())
			return it->second.image;
		else
			throw Error(ErrorCode::UnregisteredTexture);
	}

	static graphics::AnimRef get_anim_pixel_buffer(const internal::TextureKey& key)
	{
		auto it = internal::anims.find(key);
		if (it != internal::anims.end())
			return it->second;
		else
			throw Error(ErrorCode::UnregisteredTexture);
	}

	glm::vec2 get_texture_dimensions(const detail::ResourcePath& file, unsigned int texture_index)
	{
		return get_texture_dimensions({ .file = file, .index = texture_index });
	}

	graphics::ImageDimensions get_image_dimensions(const detail::ResourcePath& file, unsigned int texture_index)
	{
		return get_image_dimensions({ .file = file, .index = texture_index });
	}

	SmartReference<graphics::AnimDimensions> get_anim_dimensions(const detail::ResourcePath& file, unsigned int texture_index)
	{
		return get_anim_dimensions({ .file = file, .index = texture_index });
	}

	graphics::ImageRef get_image_pixel_buffer(const detail::ResourcePath& file, unsigned int texture_index)
	{
		return get_image_pixel_buffer({ .file = file, .index = texture_index });
	}

	graphics::AnimRef get_anim_pixel_buffer(const detail::ResourcePath& file, unsigned int texture_index)
	{
		return get_anim_pixel_buffer({ .file = file, .index = texture_index });
	}

	glm::vec2 get_texture_dimensions(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UnregisteredTexture);
		return get_texture_dimensions(it->second);
	}

	graphics::ImageDimensions get_image_dimensions(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UnregisteredTexture);
		return get_image_dimensions(it->second);
	}

	SmartReference<graphics::AnimDimensions> get_anim_dimensions(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UnregisteredTexture);
		return get_anim_dimensions(it->second);
	}

	graphics::ImageRef get_image_pixel_buffer(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UnregisteredTexture);
		return get_image_pixel_buffer(it->second);
	}

	graphics::AnimRef get_anim_pixel_buffer(const graphics::BindlessTextureRef& texture)
	{
		auto it = internal::textures.find_backward_iterator(texture);
		if (it == internal::textures.backward_end())
			throw Error(ErrorCode::UnregisteredTexture);
		return get_anim_pixel_buffer(it->second);
	}

	const graphics::NSVGAbstract& get_nsvg_abstract(const detail::ResourcePath& file)
	{
		auto it = internal::nsvg_abstracts.find(file);
		if (it != internal::nsvg_abstracts.end())
			return it->second;
		throw Error(ErrorCode::UnregisteredNsvgAbstract);
	}

	void free_texture(const detail::ResourcePath& file, unsigned int texture_index)
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

	void free_svg_texture(const detail::ResourcePath& file, unsigned int texture_index)
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

	void free_nsvg_abstract(const detail::ResourcePath& file)
	{
		auto it = internal::nsvg_abstracts.find(file);
		if (it != internal::nsvg_abstracts.end())
			internal::nsvg_abstracts.erase(it);
	}
}
