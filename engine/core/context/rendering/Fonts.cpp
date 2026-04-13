#include "Fonts.h"

#include "core/util/MetaSplitter.h"
#include "core/util/Loader.h"

#include "core/base/Errors.h"
#include "core/util/LoggerOperators.h"
#include "core/context/rendering/Textures.h"
#include "core/base/Definitions.h"

#include ".gen/keys/Font.inl"

#include ".gen/enums/rendering/text/FontStyle.inl"
#include ".gen/enums/rendering/text/CommonBufferPreset.inl"
#include ".gen/enums/rendering/texture/MagFilter.inl"
#include ".gen/enums/rendering/texture/MinFilter.inl"
#include ".gen/enums/StorageMode.inl"

// TODO v7 put actual loading logic in load/overload methods

namespace oly::context
{
	namespace internal
	{
		std::unordered_map<ResourcePath, rendering::FontFaceRef> font_faces;

		struct FontAtlasKey
		{
			ResourcePath file;
			unsigned int index;

			bool operator==(const FontAtlasKey&) const = default;
		};

		struct FontAtlasHash
		{
			size_t operator()(const FontAtlasKey& k) const { return std::hash<ResourcePath>{}(k.file) ^ std::hash<unsigned int>{}(k.index); }
		};

		std::unordered_map<FontAtlasKey, rendering::FontAtlasRef, FontAtlasHash> font_atlases;

		std::unordered_map<ResourcePath, rendering::RasterFontRef> raster_fonts;

		std::unordered_map<ResourcePath, rendering::FontFamilyRef> font_families;
	}

	struct FontsOnTerminate
	{
		void operator()() const
		{
			internal::font_faces.clear();
			internal::font_atlases.clear();
			internal::raster_fonts.clear();
			internal::font_families.clear();
		}
	};

	void internal::init_fonts(TOMLNode)
	{
		SingletonTickService<TickPhase::None, void, TerminatePhase::Graphics, FontsOnTerminate>::instance();
	}

	static utf::Codepoint parse_codepoint(const StringParam& s)
	{
		if (s.size() >= 3)
		{
			StringParam prefix = s.substr(0, 2);
			if (prefix == "U+" || prefix == "0x" || prefix == "0X" || prefix == "\\u" || prefix == "\\U" || prefix == "0h")
				return utf::Codepoint(s.substr(2).to_int(16));
			else if (s.substr(0, 3) == "&#x" && s.ends_with(';'))
				return utf::Codepoint(s.substr(3, s.size() - 3 - 1).to_int(16));
			else
				return utf::Codepoint(0);
		}
		else if (s.empty() || s.size() == 2)
			return utf::Codepoint(0);
		else
			return utf::Codepoint(s.front());
	}

	static rendering::Kerning parse_kerning(TOMLNode node)
	{
		auto kerning_arr = io::parse_key(node, _gen::keys::Font::Kerning).as_array();
		if (!kerning_arr)
			return {};

		rendering::Kerning kerning;

		size_t _k_idx = 0;
		kerning_arr->for_each([&kerning, &_k_idx](auto&& _node) {
			try
			{
				const size_t k_idx = _k_idx++;
				TOMLNode node = (TOMLNode)_node;
				const auto pair = io::parse_required<TOMLArray>(node, _gen::keys::Font::CodepointPair, { "in kerning #", k_idx });
				if (pair->size() != 2)
				{
					_OLY_ENGINE_LOG_ERROR("CONTEXT") << io::key_string(_gen::keys::Font::CodepointPair) << " field is not a 2-element array in kerning #" << k_idx << LOG.endl;
					throw Error(ErrorCode::LoadAsset);
				}
				const auto dist = io::parse_required<int>(node, _gen::keys::Font::CodepointDistance, { "in kerning #", k_idx });

				auto tc0 = pair->get_as<std::string>(0);
				auto tc1 = pair->get_as<std::string>(1);
				if (!tc0 || !tc1)
				{
					_OLY_ENGINE_LOG_ERROR("CONTEXT") << io::key_string(_gen::keys::Font::CodepointPair) << " field is not a 2-element array of strings in kerning #" << k_idx << LOG.endl;
					throw Error(ErrorCode::LoadAsset);
				}

				utf::Codepoint c1 = parse_codepoint(tc0->get());
				utf::Codepoint c2 = parse_codepoint(tc1->get());
				if (c1 && c2)
					kerning.map.emplace(std::make_pair(c1, c2), dist);
				else
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "cannot parse pair codepoints: (\"" << tc0 << "\", \"" << tc1 << "\") in kerning #" << k_idx << LOG.nl;
			}
			catch (const Error& e)
			{
				if (e.code != ErrorCode::LoadAsset)
					throw;
			}
			});

		return kerning;
	}

	rendering::FontFaceRef load_font_face(const ResourcePath& file)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto it = internal::font_faces.find(file);
		if (it != internal::font_faces.end())
			return it->second;

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "Parsing font face [" << file << "]..." << LOG.nl;

		ResourcePath import_file = file.get_import_path();
		// TODO v7 abstract away the error handling on meta.has_type()
		if (!io::MetaSplitter::meta(import_file).has_type("font"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain font type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto table = io::load_toml(import_file);
		auto node = io::parse_required_node((TOMLNode)table, _gen::keys::Font::FontFace);

		rendering::FontFaceRef font_face(file, parse_kerning(node));

		if (io::parse_optional_enum<_gen::StorageMode>(node, _gen::keys::Font::Storage, StorageMode::Discard) == StorageMode::Keep)
			internal::font_faces.emplace(file, font_face);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Font face [" << file << "] parsed" << LOG.nl;

		return font_face;
	}

	rendering::FontAtlasRef load_font_atlas(const ResourcePath& file, unsigned int index)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		internal::FontAtlasKey key{ .file = file, .index = index };
		auto it = internal::font_atlases.find(key);
		if (it != internal::font_atlases.end())
			return it->second;

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "Parsing font atlas [" << file << "]..." << LOG.nl;

		ResourcePath import_file = file.get_import_path();
		if (!io::MetaSplitter::meta(import_file).has_type("font"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain font type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto table = io::load_toml(import_file);
		TOMLNode toml = (TOMLNode)table;

		// TODO v7 associate certain keys with certain data types to automatically call the correct outer io parse function
		const auto font_atlas_list = io::parse_required<TOMLArray>(toml, _gen::keys::Font::FontAtlasArray);
		if (index >= font_atlas_list->size())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Font atlas index (" << index
				<< ") out of range for " << io::key_string(_gen::keys::Font::FontAtlasArray) << " array field of size (" << font_atlas_list->size() << ")" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto node = (TOMLNode)*font_atlas_list->get(index);

		rendering::FontOptions options;

		options.font_size = io::parse_required<float>(node, _gen::keys::Font::FontSize);
		options.min_filter = io::parse_required_enum<_gen::rendering::texture::MinFilter>(node, _gen::keys::Font::MinFilter);
		options.mag_filter = io::parse_required_enum<_gen::rendering::texture::MagFilter>(node, _gen::keys::Font::MagFilter);
		io::try_parse_if_exists(node, _gen::keys::Font::GenerateMipmaps, options.auto_generate_mipmaps);

		utf::String common_buffer = rendering::glyphs::COMMON;
		if (io::parse_if_exists_or(node, _gen::keys::Font::UseCommonBufferPreset, true))
		{
			if (auto common_buffer_preset = io::parse_enum<_gen::rendering::text::CommonBufferPreset>(node, _gen::keys::Font::CommonBufferPreset))
			{
				switch (*common_buffer_preset)
				{
				case rendering::glyphs::CommonBufferPreset::Common:
					break; // already initialized to common
				case rendering::glyphs::CommonBufferPreset::AlphaNumeric:
					common_buffer = rendering::glyphs::ALPHA_NUMERIC;
					break;
				case rendering::glyphs::CommonBufferPreset::Numeric:
					common_buffer = rendering::glyphs::NUMERIC;
					break;
				case rendering::glyphs::CommonBufferPreset::Alphabet:
					common_buffer = rendering::glyphs::ALPHABET;
					break;
				case rendering::glyphs::CommonBufferPreset::AlphabetLowercase:
					common_buffer = rendering::glyphs::ALPHABET_LOWERCASE;
					break;
				case rendering::glyphs::CommonBufferPreset::AlphabetUppercase:
					common_buffer = rendering::glyphs::ALPHABET_UPPERCASE;
					break;
				}
			}
		}
		else if (auto _common_buffer = io::parse_key(node, _gen::keys::Font::CommonBuffer).value<std::string>())
			common_buffer = _common_buffer.value();

		rendering::FontAtlasRef font_atlas(context::load_font_face(file), options, common_buffer);
		if (io::parse_optional_enum<_gen::StorageMode>(node, _gen::keys::Font::Storage, StorageMode::Discard) == StorageMode::Keep)
			internal::font_atlases.emplace(key, font_atlas);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Font atlas [" << file << "] at index #" << index << " parsed" << LOG.nl;

		return font_atlas;
	}

	rendering::RasterFontRef load_raster_font(const ResourcePath& file)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto it = internal::raster_fonts.find(file);
		if (it != internal::raster_fonts.end())
			return it->second;

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "Parsing raster font [" << file << "]..." << LOG.nl;

		auto meta = io::MetaSplitter::meta(file);
		if (!meta.has_type("raster_font"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain raster font type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto table = io::load_toml(file);
		TOMLNode toml = (TOMLNode)table;

		const auto space_advance_width = io::parse_required<float>(toml, _gen::keys::Font::SpaceAdvanceWidth);
		const auto line_height = io::parse_required<float>(toml, _gen::keys::Font::LineHeight);
		const auto font_scale = io::parse_optional(toml, _gen::keys::Font::FontScale, glm::vec2(1.0f));

		std::vector<std::string> texture_files;
		if (auto a = io::parse_key(toml, _gen::keys::Font::TextureFileArray).as_array())
		{
			texture_files.reserve(a->size());
			for (size_t i = 0; i < a->size(); ++i)
			{
				if (auto texture_file = a->get_as<std::string>(i))
					texture_files.push_back(texture_file->get());
				else
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Invalid entry in " << io::key_string(_gen::keys::Font::TextureFileArray) << " array" << LOG.nl;
			}
		}

		std::unordered_map<utf::Codepoint, rendering::RasterFontGlyph> glyphs;
		if (auto glyph_array = io::parse_key(toml, _gen::keys::Font::GlyphArray).as_array())
		{
			glyph_array->for_each([&glyphs, &texture_files](auto&& _g) {
				TOMLNode g = (TOMLNode)_g;

				utf::Codepoint codepoint = utf::Codepoint(0);
				if (auto v = io::parse_key(g, _gen::keys::Font::Codepoint).value<std::string>())
					codepoint = parse_codepoint(v.value());
				if (codepoint == utf::Codepoint(0))
				{
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Cannot parse " << io::key_string(_gen::keys::Font::Codepoint) << " field in glyphs array, skipping glyph..." << LOG.nl;
					return;
				}

				std::string texture_file;
				unsigned int tidx = io::parse_or(io::parse_key(g, _gen::keys::Font::TextureFile), 0u);
				if (tidx < texture_files.size())
					texture_file = texture_files[tidx];
				else
				{
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Texture file indexer (" << tidx
						<< ") is out of range (" << texture_files.size() << "), skipping glyph..." << LOG.nl;
					return;
				}

				unsigned int texture_index = io::parse_or(io::parse_key(g, _gen::keys::Font::TextureIndex), 0u);

				math::IRect2D location = math::IRect2D::load(io::parse_key(g, _gen::keys::Font::Location));
				if (location.x2 <= location.x1 || location.y2 <= location.y1)
				{
					_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Cannot parse valid " << io::key_string(_gen::keys::Font::Location) << " field, skipping glyph..." << LOG.nl;
					return;
				}

				math::TopSidePadding padding = math::TopSidePadding::load(io::parse_key(g, _gen::keys::Font::Padding));

				math::PositioningMode origin_offset_mode = math::PositioningMode::load(io::parse_key(g, _gen::keys::Font::OriginOffsetMode), math::PositioningMode::RELATIVE);

				glm::vec2 origin_offset = {};
				io::try_parse(io::parse_key(g, _gen::keys::Font::OriginOffset), origin_offset);

				glyphs.emplace(codepoint, rendering::RasterFontGlyph(context::load_texture(texture_file, texture_index), location, padding, origin_offset_mode, origin_offset));
				});
		}

		rendering::RasterFontRef raster_font(std::move(glyphs), space_advance_width, line_height, font_scale, parse_kerning(toml));
		if (io::parse_optional_enum<_gen::StorageMode>(toml, _gen::keys::Font::Storage, StorageMode::Discard) == StorageMode::Keep)
			internal::raster_fonts.emplace(file, raster_font);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Raster font [" << file << "] parsed" << LOG.nl;

		return raster_font;
	}

	rendering::FontFamilyRef load_font_family(const ResourcePath& file)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto it = internal::font_families.find(file);
		if (it != internal::font_families.end())
			return it->second;

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "Parsing font family [" << file << "]..." << LOG.nl;

		if (!io::MetaSplitter::meta(file).has_type("font_family"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain font family type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto table = io::load_toml(file);
		TOMLNode toml = (TOMLNode)table;

		rendering::FontFamilyRef font_family = REF_INIT;
		if (auto a = io::parse_key(toml, _gen::keys::Font::StyleArray).as_array())
		{
			a->for_each([&file, &styles = font_family->styles](auto&& _node) {
				try
				{
					TOMLNode node = (TOMLNode)_node;

					rendering::FontStyle style = io::parse_optional_enum<_gen::rendering::text::FontStyle>(node, _gen::keys::Font::Style, rendering::FontStyle::Regular);

					ResourcePath font_file(io::parse_required<std::string>(node, _gen::keys::Font::File, { "from font family style" }), file);
					rendering::FontFamily::FontRef font;
					if (font_file.is_import_path())
					{
						auto meta = io::MetaSplitter::meta(font_file);
						if (meta.has_type("raster_font"))
							font = context::load_raster_font(font_file);
						else
						{
							std::optional<std::string> type = meta.get_type();
							_OLY_ENGINE_LOG_WARNING("CONTEXT") << font_file << " has unrecognized meta type: \"" << (type ? *type : "") << "\"" << LOG.nl;
							return;
						}
					}
					else
						font = context::load_font_atlas(font_file, io::parse_or(io::parse_key(node, _gen::keys::Font::AtlasIndex), 0u));

					styles.emplace(style, std::move(font));
				}
				catch (const Error& e)
				{
					if (e.code != ErrorCode::LoadAsset)
						throw;
				}
				});
		}

		if (io::parse_optional_enum<_gen::StorageMode>(toml, _gen::keys::Font::Storage, StorageMode::Discard) == StorageMode::Keep)
			internal::font_families.emplace(file, font_family);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Font family [" << file << "] parsed" << LOG.nl;

		return font_family;
	}

	void free_font_face(const ResourcePath& file)
	{
		internal::font_faces.erase(file);
	}

	void free_font_atlas(const ResourcePath& file, unsigned int index)
	{
		internal::font_atlases.erase({ file, index });
	}

	void free_raster_font(const ResourcePath& file)
	{
		internal::raster_fonts.erase(file);
	}

	void free_font_family(const ResourcePath& file)
	{
		internal::font_families.erase(file);
	}

	rendering::FontSelection load_font_selection(const ResourcePath& font_family, rendering::FontStyle style)
	{
		rendering::FontSelection font{ .family = load_font_family(font_family), .style = style };
		if (!font.style_exists())
			_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Font style (" << (unsigned int)style << ") not supported by family " << font_family << LOG.nl;
		return font;
	}

	rendering::Font load_font(const ResourcePath& file, unsigned int index)
	{
		if (file.is_import_path())
		{
			auto meta = io::MetaSplitter::meta(file);
			if (meta.has_type("raster_font"))
				return load_raster_font(file);
			else if (meta.has_type("font_family"))
				return load_font_selection(file, _gen::rendering::text::FontStyle::val(index));
			else
			{
				std::optional<std::string> type = meta.get_type();
				_OLY_ENGINE_LOG_ERROR("CONTEXT") << file << " has unrecognized meta type: \"" << (type ? *type : "") << "\"" << LOG.nl;
				throw Error(ErrorCode::LoadAsset);
			}
		}
		else
			return load_font_atlas(file, index);
	}
}
