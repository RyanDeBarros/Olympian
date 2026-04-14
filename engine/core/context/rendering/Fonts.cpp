#include "Fonts.h"

#include "core/util/MetaSplitter.h"
#include "core/util/Loader.h"
#include "core/util/Parser.h"

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
#include ".gen/enums/PositioningMode.inl"

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
		auto kerning_arr = assets::Parser(node).optional<TOMLArray>(_gen::keys::Font::Kerning)();
		if (!kerning_arr)
			return {};

		rendering::Kerning kerning;

		size_t _k_idx = 0;
		kerning_arr->for_each([&kerning, &_k_idx](auto&& node) {
			try
			{
				const size_t k_idx = _k_idx++;
				assets::Parser parser((TOMLNode)node, { "in kerning #", k_idx });
				const auto pair = parser.required<TOMLArray>(_gen::keys::Font::CodepointPair)({ .min_size = 2, .max_size = 2 });
				const auto dist = parser.required<int>(_gen::keys::Font::CodepointDistance)();

				auto tc0 = pair->get_as<std::string>(0);
				auto tc1 = pair->get_as<std::string>(1);
				if (!tc0 || !tc1)
				{
					// TODO v7 somehow auto-log/throw this in parser?
					_OLY_ENGINE_LOG_ERROR("CONTEXT") << assets::key_string(_gen::keys::Font::CodepointPair) << " field is not a 2-element array of strings in kerning #" << k_idx << LOG.endl;
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

		auto toml = io::load_toml(import_file);
		assets::Parser parser(toml);

		auto node = parser.required<TOMLNode>(_gen::keys::Font::FontFace)();

		rendering::FontFaceRef font_face(file, parse_kerning(node));

		if (parser.translate<_gen::StorageMode>().defaulted(_gen::keys::Font::Storage)(StorageMode::Discard) == StorageMode::Keep)
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

		auto toml = io::load_toml(import_file);

		// TODO v7 associate certain keys with certain data types to automatically call the correct outer io parse function

		const auto font_atlas_list = assets::Parser(toml).required<TOMLArray>(_gen::keys::Font::FontAtlasArray)({ .min_size = index + 1 });

		auto node = (TOMLNode)*font_atlas_list->get(index);
		assets::Parser parser(node);

		rendering::FontOptions options;

		parser.required(_gen::keys::Font::FontSize)(options.font_size);
		options.min_filter = parser.translate<_gen::rendering::texture::MinFilter>().required(_gen::keys::Font::MinFilter)();
		options.mag_filter = parser.translate<_gen::rendering::texture::MagFilter>().required(_gen::keys::Font::MagFilter)();
		parser.optional(_gen::keys::Font::GenerateMipmaps)(options.auto_generate_mipmaps);

		utf::String common_buffer = rendering::glyphs::COMMON;
		if (parser.defaulted(_gen::keys::Font::UseCommonBufferPreset)(true))
		{
			if (auto common_buffer_preset = parser.translate<_gen::rendering::text::CommonBufferPreset>().optional(_gen::keys::Font::CommonBufferPreset)())
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
		else if (auto _common_buffer = parser.optional<std::string>(_gen::keys::Font::CommonBuffer)())
			common_buffer = *_common_buffer;

		rendering::FontAtlasRef font_atlas(context::load_font_face(file), options, common_buffer);
		if (parser.translate<_gen::StorageMode>().defaulted(_gen::keys::Font::Storage)(StorageMode::Discard) == StorageMode::Keep)
			internal::font_atlases.emplace(key, font_atlas);

		// TODO v7 add Trace log level for stuff like this, that is lower than Debug
		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Font atlas [" << file << "] at index #" << index << " parsed" << LOG.nl;

		return font_atlas;
	}

	rendering::RasterFontRef load_raster_font(const ResourcePath& file)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.endl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto it = internal::raster_fonts.find(file);
		if (it != internal::raster_fonts.end())
			return it->second;

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "Parsing raster font [" << file << "]..." << LOG.nl;

		auto meta = io::MetaSplitter::meta(file);
		if (!meta.has_type("raster_font"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain raster font type" << LOG.endl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto table = io::load_toml(file);
		TOMLNode toml = (TOMLNode)table;
		assets::Parser parser(toml);

		const auto space_advance_width = parser.required<float>(_gen::keys::Font::SpaceAdvanceWidth)();
		const auto line_height = parser.required<float>(_gen::keys::Font::LineHeight)();
		const auto font_scale = parser.defaulted(_gen::keys::Font::FontScale)(glm::vec2(1.0f));
		const auto texture_files = parser.defaulted<std::vector<std::string>>(_gen::keys::Font::TextureFileArray)();

		std::unordered_map<utf::Codepoint, rendering::RasterFontGlyph> glyphs;
		if (auto glyph_array = parser.optional<TOMLArray>(_gen::keys::Font::GlyphArray)())
		{
			glyph_array->for_each([&glyphs, &texture_files](auto&& g) {
				assets::Parser parser((TOMLNode)g);

				utf::Codepoint codepoint = utf::Codepoint(0);
				if (auto v = parser.optional<std::string>(_gen::keys::Font::Codepoint)())
					codepoint = parse_codepoint(*v);
				if (codepoint == utf::Codepoint(0))
				{
					_OLY_ENGINE_LOG_ERROR("CONTEXT") << "codepoint field is zero, skipping glyph..." << LOG.endl;
					throw Error(ErrorCode::LoadAsset);
				}

				std::string texture_file;
				unsigned int tidx = parser.defaulted(_gen::keys::Font::TextureFile)(0u);
				if (tidx < texture_files.size())
					texture_file = texture_files[tidx];
				else
				{
					_OLY_ENGINE_LOG_ERROR("CONTEXT") << "texture file indexer (" << tidx << ") is out of range (" << texture_files.size() << "), skipping glyph..." << LOG.endl;
					throw Error(ErrorCode::LoadAsset);
				}

				unsigned int texture_index = parser.defaulted(_gen::keys::Font::TextureIndex)(0u);

				math::IRect2D location = math::IRect2D::load(parser.field(_gen::keys::Font::Location), true);
				math::TopSidePadding padding = math::TopSidePadding::load(parser.field(_gen::keys::Font::Padding));
				math::PositioningMode origin_offset_mode = parser.translate<_gen::PositioningMode>().defaulted(_gen::keys::Font::OriginOffsetMode)(math::PositioningMode::Relative);
				glm::vec2 origin_offset = parser.defaulted<glm::vec2>(_gen::keys::Font::OriginOffset)();

				glyphs.emplace(codepoint, rendering::RasterFontGlyph(context::load_texture(texture_file, texture_index), location, padding, origin_offset_mode, origin_offset));
				});
		}

		rendering::RasterFontRef raster_font(std::move(glyphs), space_advance_width, line_height, font_scale, parse_kerning(toml));
		if (parser.translate<_gen::StorageMode>().defaulted(_gen::keys::Font::Storage)(StorageMode::Discard) == StorageMode::Keep)
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

		auto toml = io::load_toml(file);
		assets::Parser parser(toml);


		rendering::FontFamilyRef font_family = REF_INIT;
		if (auto a = parser.optional<TOMLArray>(_gen::keys::Font::StyleArray)())
		{
			a->for_each([&file, &styles = font_family->styles](auto&& node) {
				try
				{
					assets::Parser parser((TOMLNode)node);

					rendering::FontStyle style = parser.translate<_gen::rendering::text::FontStyle>().defaulted(_gen::keys::Font::Style)(rendering::FontStyle::Regular);

					ResourcePath font_file(parser.required<std::string>(_gen::keys::Font::File)(), file);
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
						font = context::load_font_atlas(font_file, parser.defaulted(_gen::keys::Font::AtlasIndex)(0u));

					styles.emplace(style, std::move(font));
				}
				catch (const Error& e)
				{
					if (e.code != ErrorCode::LoadAsset)
						throw;
				}
				});
		}

		if (parser.translate<_gen::StorageMode>().defaulted(_gen::keys::Font::Storage)(StorageMode::Discard) == StorageMode::Keep)
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
