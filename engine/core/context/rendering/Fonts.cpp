#include "Fonts.h"

#include "core/util/Loader.h"
#include "core/util/Parser.h"

#include "core/base/Errors.h"
#include "core/util/LoggerOperators.h"
#include "core/context/rendering/Textures.h"

#include "assets/MetaSplitter.h"
#include "definitions/Keys.h"
#include "definitions/enums/StorageMode.h"

// TODO v8 put actual loading logic in load/overload methods

namespace oly::context
{
	namespace internal
	{
		std::unordered_map<detail::ResourcePath, rendering::FontFaceRef> font_faces;

		struct FontAtlasKey
		{
			detail::ResourcePath file;
			unsigned int index;

			bool operator==(const FontAtlasKey&) const = default;
		};

		struct FontAtlasHash
		{
			size_t operator()(const FontAtlasKey& k) const { return std::hash<detail::ResourcePath>{}(k.file) ^ std::hash<unsigned int>{}(k.index); }
		};

		std::unordered_map<FontAtlasKey, rendering::FontAtlasRef, FontAtlasHash> font_atlases;

		std::unordered_map<detail::ResourcePath, rendering::RasterFontRef> raster_fonts;

		std::unordered_map<detail::ResourcePath, rendering::FontFamilyRef> font_families;
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

	static auto make_nonnull_codepoint_validator()
	{
		return assets::make_single_validator<utf::Codepoint>([](utf::Codepoint c) { return c != utf::Codepoint(0); },
			[](utf::Codepoint c) { return DeferredStringList{ "-> null codepoint not allowed" }; });
	}

	static rendering::Kerning parse_kerning(TOMLNode node)
	{
		auto kerning_arr = assets::Parser(node).optional<TOMLArray>(detail::Key::Kerning)();
		if (!kerning_arr)
			return {};

		rendering::Kerning kerning;

		size_t _k_idx = 0;
		kerning_arr->for_each([&kerning, &_k_idx](auto&& node) {
			try
			{
				const size_t k_idx = _k_idx++;
				assets::Parser parser((TOMLNode)node, { "in kerning #", k_idx });
				const auto pair = parser.required<std::array<utf::Codepoint, 2>>(detail::Key::CodepointPair, make_nonnull_codepoint_validator())();
				const auto dist = parser.required<int>(detail::Key::CodepointDistance)();
				kerning.map.emplace(std::make_pair(pair.at(0), pair.at(1)), dist);
			}
			catch (const Error& e)
			{
				if (e.code != ErrorCode::LoadAsset)
					throw;
			}
			});

		return kerning;
	}

	rendering::FontFaceRef load_font_face(const detail::ResourcePath& file)
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

		detail::ResourcePath import_file = file.get_import_path();
		// TODO v8 abstract away the error handling on meta.has_type()
		if (!detail::MetaSplitter::decode_meta(import_file).has_type(detail::Key::Meta_Font))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain font type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto toml = io::load_toml(import_file);
		assets::Parser parser(toml);

		auto node = parser.required<TOMLNode>(detail::Key::FontFace)();

		rendering::FontFaceRef font_face(file, parse_kerning(node));

		if (parser.defaulted(detail::Key::Storage)(detail::StorageMode::Discard) == detail::StorageMode::Keep)
			internal::font_faces.emplace(file, font_face);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Font face [" << file << "] parsed" << LOG.nl;

		return font_face;
	}

	rendering::FontAtlasRef load_font_atlas(const detail::ResourcePath& file, unsigned int index)
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

		detail::ResourcePath import_file = file.get_import_path();
		if (!detail::MetaSplitter::decode_meta(import_file).has_type(detail::Key::Meta_Font))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain font type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto toml = io::load_toml(import_file);

		const auto font_atlas_list = assets::Parser(toml).required<TOMLArray>(detail::Key::FontAtlasArray, assets::make_single_validator<TOMLArray>(
			[index](const TOMLArray arr) { return index < arr->size(); },
			[index](const TOMLArray arr) { return DeferredStringList{ "-> array size (", std::to_string(arr->size()), ") is not larger than font index (", std::to_string(index), ")" }; }
		))();

		auto node = (TOMLNode)*font_atlas_list->get(index);
		assets::Parser parser(node);

		rendering::FontOptions options;

		parser.required(detail::Key::FontSize)(options.font_size);
		parser.required(detail::Key::MinFilter)(options.min_filter);
		parser.required(detail::Key::MagFilter)(options.mag_filter);
		parser.optional(detail::Key::GenerateMipmaps)(options.auto_generate_mipmaps);

		utf::String common_buffer = rendering::glyphs::COMMON;
		if (parser.defaulted(detail::Key::UseCommonBufferPreset)(true))
		{
			if (auto common_buffer_preset = parser.optional<rendering::glyphs::CommonBufferPreset>(detail::Key::CommonBufferPreset)())
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
		else if (auto _common_buffer = parser.optional<std::string>(detail::Key::CommonBuffer)())
			common_buffer = *_common_buffer;

		rendering::FontAtlasRef font_atlas(context::load_font_face(file), options, common_buffer);
		if (parser.defaulted(detail::Key::Storage)(detail::StorageMode::Discard) == detail::StorageMode::Keep)
			internal::font_atlases.emplace(key, font_atlas);

		// TODO v8 add Trace log level for stuff like this, that is lower than Debug
		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Font atlas [" << file << "] at index #" << index << " parsed" << LOG.nl;

		return font_atlas;
	}

	rendering::RasterFontRef load_raster_font(const detail::ResourcePath& file)
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

		auto meta = detail::MetaSplitter::decode_meta(file.get_absolute());
		if (!meta.has_type(detail::Key::Meta_RasterFont))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain raster font type" << LOG.endl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto table = io::load_toml(file);
		TOMLNode toml = (TOMLNode)table;
		assets::Parser parser(toml);

		const auto space_advance_width = parser.required<float>(detail::Key::SpaceAdvanceWidth)();
		const auto line_height = parser.required<float>(detail::Key::LineHeight)();
		const auto font_scale = parser.defaulted(detail::Key::FontScale)(glm::vec2(1.0f));
		const auto texture_files = parser.defaulted<std::vector<std::string>>(detail::Key::TextureFileArray)();

		std::unordered_map<utf::Codepoint, rendering::RasterFontGlyph> glyphs;
		if (auto glyph_array = parser.optional<TOMLArray>(detail::Key::GlyphArray)())
		{
			glyph_array->for_each([&glyphs, &texture_files](auto&& g) {
				try
				{
					assets::Parser parser((TOMLNode)g);

					utf::Codepoint codepoint = parser.required<utf::Codepoint>(detail::Key::Codepoint, make_nonnull_codepoint_validator())();

					std::string texture_file;
					unsigned int tidx = parser.defaulted(detail::Key::TextureFile, assets::make_single_validator<unsigned int>(
						[sz = texture_files.size()](unsigned int v) { return v < sz; },
						[sz = texture_files.size()](unsigned int v) { return DeferredStringList{ "-> (", std::to_string(v), ") out of range (", std::to_string(sz), "), skipping glyph..."}; }
					))(0u);
					texture_file = texture_files[tidx];

					unsigned int texture_index = parser.defaulted(detail::Key::TextureIndex)(0u);

					math::IRect2D location = math::IRect2D::load(parser.field(detail::Key::Location), true);
					math::TopSidePadding padding = math::TopSidePadding::load(parser.field(detail::Key::Padding));
					math::PositioningMode origin_offset_mode = parser.defaulted(detail::Key::OriginOffsetMode)(math::PositioningMode::Relative);
					glm::vec2 origin_offset = parser.defaulted<glm::vec2>(detail::Key::OriginOffset)();

					glyphs.emplace(codepoint, rendering::RasterFontGlyph(context::load_texture(texture_file, texture_index), location, padding, origin_offset_mode, origin_offset));
				}
				catch (const Error& e)
				{
					if (e.code != ErrorCode::LoadAsset)
						throw;
				}
				});
		}

		rendering::RasterFontRef raster_font(std::move(glyphs), space_advance_width, line_height, font_scale, parse_kerning(toml));
		if (parser.defaulted(detail::Key::Storage)(detail::StorageMode::Discard) == detail::StorageMode::Keep)
			internal::raster_fonts.emplace(file, raster_font);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Raster font [" << file << "] parsed" << LOG.nl;

		return raster_font;
	}

	rendering::FontFamilyRef load_font_family(const detail::ResourcePath& file)
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

		if (!detail::MetaSplitter::decode_meta(file.get_absolute()).has_type(detail::Key::Meta_FontFamily))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain font family type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto toml = io::load_toml(file);
		assets::Parser parser(toml);


		rendering::FontFamilyRef font_family = REF_INIT;
		if (auto a = parser.optional<TOMLArray>(detail::Key::StyleArray)())
		{
			a->for_each([&file, &styles = font_family->styles](auto&& node) {
				try
				{
					assets::Parser parser((TOMLNode)node);

					rendering::FontStyle style = parser.defaulted(detail::Key::Style)(rendering::FontStyle::Regular);

					detail::ResourcePath font_file(parser.required<std::string>(detail::Key::File)(), file);
					rendering::FontFamily::FontRef font;
					if (font_file.is_import_path())
					{
						auto meta = detail::MetaSplitter::decode_meta(font_file.get_absolute());
						if (meta.has_type(detail::Key::Meta_RasterFont))
							font = context::load_raster_font(font_file);
						else
						{
							std::optional<detail::Key> type = meta.get_type();
							_OLY_ENGINE_LOG_WARNING("CONTEXT") << font_file << " has unrecognized meta type: \"" << (type ? detail::encode_key(*type) : "") << "\"" << LOG.nl;
							return;
						}
					}
					else
						font = context::load_font_atlas(font_file, parser.defaulted(detail::Key::AtlasIndex)(0u));

					styles.emplace(style, std::move(font));
				}
				catch (const Error& e)
				{
					if (e.code != ErrorCode::LoadAsset)
						throw;
				}
				});
		}

		if (parser.defaulted(detail::Key::Storage)(detail::StorageMode::Discard) == detail::StorageMode::Keep)
			internal::font_families.emplace(file, font_family);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Font family [" << file << "] parsed" << LOG.nl;

		return font_family;
	}

	void free_font_face(const detail::ResourcePath& file)
	{
		internal::font_faces.erase(file);
	}

	void free_font_atlas(const detail::ResourcePath& file, unsigned int index)
	{
		internal::font_atlases.erase({ file, index });
	}

	void free_raster_font(const detail::ResourcePath& file)
	{
		internal::raster_fonts.erase(file);
	}

	void free_font_family(const detail::ResourcePath& file)
	{
		internal::font_families.erase(file);
	}

	rendering::FontSelection load_font_selection(const detail::ResourcePath& font_family, rendering::FontStyle style)
	{
		rendering::FontSelection font{ .family = load_font_family(font_family), .style = style };
		if (!font.style_exists())
			_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Font style (" << (unsigned int)style << ") not supported by family " << font_family << LOG.nl;
		return font;
	}

	rendering::Font load_font(const detail::ResourcePath& file, unsigned int index)
	{
		if (file.is_import_path())
		{
			auto meta = detail::MetaSplitter::decode_meta(file.get_absolute());
			if (meta.has_type(detail::Key::Meta_RasterFont))
				return load_raster_font(file);
			else if (meta.has_type(detail::Key::Meta_FontFamily))
				return load_font_selection(file, static_cast<rendering::FontStyle::Mode>(index));
			else
			{
				std::optional<detail::Key> type = meta.get_type();
				_OLY_ENGINE_LOG_ERROR("CONTEXT") << file << " has unrecognized meta type: \"" << (type ? detail::encode_key(*type) : "") << "\"" << LOG.nl;
				throw Error(ErrorCode::LoadAsset);
			}
		}
		else
			return load_font_atlas(file, index);
	}
}
