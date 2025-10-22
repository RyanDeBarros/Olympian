#include "Fonts.h"

#include "registries/MetaSplitter.h"
#include "registries/Loader.h"

#include "core/base/Errors.h"
#include "core/util/LoggerOperators.h"
#include "core/context/rendering/Textures.h"

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

	void internal::terminate_fonts()
	{
		internal::font_faces.clear();
		internal::font_atlases.clear();
		internal::raster_fonts.clear();
		internal::font_families.clear();
	}

	static utf::Codepoint parse_codepoint(const std::string& s)
	{
		if (s.size() >= 3)
		{
			std::string prefix = s.substr(0, 2);
			if (prefix == "U+" || prefix == "0x" || prefix == "0X" || prefix == "\\u" || prefix == "\\U" || prefix == "0h")
				return utf::Codepoint(std::stoi(s.substr(2), nullptr, 16));
			else if (s.substr(0, 3) == "&#x" && s.ends_with(";"))
				return utf::Codepoint(std::stoi(s.substr(3, s.size() - 3 - 1), nullptr, 16));
			else
				return utf::Codepoint(0);
		}
		else if (s.empty() || s.size() == 2)
			return utf::Codepoint(0);
		else
			return utf::Codepoint(s[0]);
	}

	static rendering::Kerning parse_kerning(TOMLNode node)
	{
		auto kerning_arr = node["kerning"].as_array();
		if (!kerning_arr)
			return {};

		rendering::Kerning kerning;

		size_t _k_idx = 0;
		kerning_arr->for_each([&kerning, &_k_idx](auto&& node) {
			const size_t k_idx = _k_idx++;
			if constexpr (toml::is_table<decltype(node)>)
			{
				auto pair = node["pair"].as_array();
				if (!pair)
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In kerning #" << k_idx << " - missing \"pair\" array field." << LOG.nl;
					return;
				}
				if (pair->size() != 2)
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In kerning #" << k_idx << " - \"pair\" field is not a 2-element array." << LOG.nl;
					return;
				}
				int dist = 0;
				if (!reg::parse_int(node["dist"], dist))
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In kerning #" << k_idx << " - missing \"dist\" int field." << LOG.nl;
					return;
				}

				auto tc0 = pair->get_as<std::string>(0);
				auto tc1 = pair->get_as<std::string>(1);
				if (!tc0 || !tc1)
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In kerning #" << k_idx << " - \"pair\" field is not a 2-element array of strings." << LOG.nl;
					return;
				}

				utf::Codepoint c1 = parse_codepoint(tc0->get());
				utf::Codepoint c2 = parse_codepoint(tc1->get());
				if (c1 && c2)
					kerning.map.emplace(std::make_pair(c1, c2), dist);
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In kerning #" << k_idx
					<< " - cannot parse pair codepoints: (\"" << tc0 << "\", \"" << tc1 << "\")." << LOG.nl;
			}
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse kerning #" << k_idx << " - not a TOML table." << LOG.nl;
			});

		return kerning;
	}

	rendering::FontFaceRef load_font_face(const ResourcePath& file)
	{
		auto it = internal::font_faces.find(file);
		if (it != internal::font_faces.end())
			return it->second;

		auto toml = reg::load_toml(file.get_import_path());
		auto node = toml["font_face"];
		if (!node.as_table())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Cannot load font face " << file << " - missing \"font_face\" table." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing font face [" << (src ? *src : "") << "]..." << LOG.nl;
		}

		rendering::FontFaceRef font_face(file, parse_kerning(node));
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			internal::font_faces.emplace(file, font_face);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Font face [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return font_face;
	}

	rendering::FontAtlasRef load_font_atlas(const ResourcePath& file, unsigned int index)
	{
		// TODO v5 add empty filename checks to all asset loaders
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		internal::FontAtlasKey key{ .file = file, .index = index };
		auto it = internal::font_atlases.find(key);
		if (it != internal::font_atlases.end())
			return it->second;

		auto toml = reg::load_toml(file.get_import_path());
		auto font_atlas_list = toml["font_atlas"].as_array();
		if (!font_atlas_list || font_atlas_list->empty())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing or empty \"font_atlas\" array field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}
		index = glm::clamp(index, (unsigned int)0, (unsigned int)font_atlas_list->size() - 1);
		auto node = TOMLNode(*font_atlas_list->get(index));

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing font atlas [" << (src ? *src : "") << "] at index #" << index << "..." << LOG.nl;
		}

		rendering::FontOptions options;

		if (!reg::parse_float(node["font_size"], options.font_size))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing \"font_size\" field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		reg::parse_min_filter(node["min_filter"], options.min_filter);
		reg::parse_mag_filter(node["mag_filter"], options.mag_filter);
		reg::parse_bool(node["generate_mipmaps"], options.auto_generate_mipmaps);

		utf::String common_buffer = rendering::glyphs::COMMON;
		if (reg::parse_bool_or(node["use_common_buffer_preset"], true))
		{
			if (auto _common_buffer_preset = node["common_buffer_preset"].value<std::string>())
			{
				const std::string& common_buffer_preset = _common_buffer_preset.value();
				if (common_buffer_preset == "common")
					; // already initialized to common
				else if (common_buffer_preset == "alpha_numeric")
					common_buffer = rendering::glyphs::ALPHA_NUMERIC;
				else if (common_buffer_preset == "numeric")
					common_buffer = rendering::glyphs::NUMERIC;
				else if (common_buffer_preset == "alphabet")
					common_buffer = rendering::glyphs::ALPHABET;
				else if (common_buffer_preset == "alphabet_lowercase")
					common_buffer = rendering::glyphs::ALPHABET_LOWERCASE;
				else if (common_buffer_preset == "alphabet_uppercase")
					common_buffer = rendering::glyphs::ALPHABET_UPPERCASE;
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized common buffer preset value \"" << common_buffer_preset << "\"." << LOG.nl;
			}
		}
		else if (auto _common_buffer = node["common_buffer"].value<std::string>())
			common_buffer = _common_buffer.value();

		rendering::FontAtlasRef font_atlas(context::load_font_face(file), options, common_buffer);
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			internal::font_atlases.emplace(key, font_atlas);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Font atlas [" << (src ? *src : "") << "] at index #" << index << " parsed." << LOG.nl;
		}

		return font_atlas;
	}

	rendering::RasterFontRef load_raster_font(const ResourcePath& file)
	{
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto it = internal::raster_fonts.find(file);
		if (it != internal::raster_fonts.end())
			return it->second;

		OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing raster font [" << file << "]..." << LOG.nl;

		auto meta = reg::MetaSplitter::meta(file); // TODO v5 use MetaSplitter throughout registries
		if (!meta.has_type("raster_font"))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Meta fields do not contain raster font type." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto table = reg::load_toml(file);
		TOMLNode toml = (TOMLNode)table;

		float space_advance_width;
		if (!reg::parse_float(toml["space_advance_width"], space_advance_width))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing \"space_advance_width\" field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		float line_height;
		if (!reg::parse_float(toml["line_height"], line_height))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing \"line_height\" field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		glm::vec2 font_scale = glm::vec2(1.0f);
		if (auto a = toml["font_scale"])
		{
			if (!reg::parse_vec(a, font_scale))
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"font_scale\" field." << LOG.nl;
		}

		std::vector<std::string> texture_files;
		if (auto a = toml["texture_files"].as_array())
		{
			texture_files.reserve(a->size());
			for (size_t i = 0; i < a->size(); ++i)
			{
				auto texture_file = a->get_as<std::string>(i);
				if (texture_file)
					texture_files.push_back(texture_file->get());
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Invalid entry in \"texture_files\" array." << LOG.nl;
			}
		}

		std::unordered_map<utf::Codepoint, rendering::RasterFontGlyph> glyphs;
		if (auto glyph_array = toml["glyphs"].as_array())
		{
			glyph_array->for_each([&glyphs, &texture_files](auto&& _g) {
				// TODO v5 use this design in visitor patterns throughout registries
				TOMLNode g = (TOMLNode)_g;

				utf::Codepoint codepoint = utf::Codepoint(0);
				if (auto v = g["codepoint"].value<std::string>())
					codepoint = parse_codepoint(v.value());
				if (codepoint == utf::Codepoint(0))
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"codepoint\" field in glyphs array, skipping glyph..." << LOG.nl;
					return;
				}

				std::string texture_file;
				unsigned int tidx = reg::parse_uint_or(g["texture_file"], 0);
				if (tidx < texture_files.size())
					texture_file = texture_files[tidx];
				else
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Texture file indexer (" << tidx
						<< ") is out of range (" << texture_files.size() << "), skipping glyph..." << LOG.nl;
					return;
				}

				unsigned int texture_index = reg::parse_uint_or(g["texture_index"], 0);

				math::IRect2D location;
				if (!reg::parse_shape(g["location"], location))
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"location\" field, skipping glyph..." << LOG.nl;
					return;
				}

				math::TopSidePadding padding = reg::parse_topside_padding(g["padding"]);

				math::PositioningMode origin_offset_mode = math::PositioningMode::RELATIVE;
				reg::parse_enum(g["origin_offset_mode"], origin_offset_mode);

				glm::vec2 origin_offset = {};
				reg::parse_vec(g["origin_offset"], origin_offset);

				glyphs.emplace(codepoint, rendering::RasterFontGlyph(context::load_texture(texture_file, texture_index), location, padding, origin_offset_mode, origin_offset));
				});
		}

		rendering::RasterFontRef raster_font(std::move(glyphs), space_advance_width, line_height, font_scale, parse_kerning(toml));
		if (toml["storage"].value<std::string>().value_or("discard") == "keep")
			internal::raster_fonts.emplace(file, raster_font);

		OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Raster font [" << file << "] parsed." << LOG.nl;

		return raster_font;
	}

	rendering::FontFamilyRef load_font_family(const ResourcePath& file)
	{
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto it = internal::font_families.find(file);
		if (it != internal::font_families.end())
			return it->second;

		OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing font family [" << file << "]..." << LOG.nl;

		if (!reg::MetaSplitter::meta(file).has_type("font_family"))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Meta fields do not contain font family type." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto table = reg::load_toml(file);
		TOMLNode toml = (TOMLNode)table;

		rendering::FontFamilyRef font_family = REF_INIT;
		if (auto a = toml["styles"].as_array())
		{
			a->for_each([&file, &styles = font_family->styles](auto&& _node) {
				TOMLNode node = (TOMLNode)_node;

				rendering::FontStyle style = rendering::FontStyle::REGULAR();
				if (!reg::parse_uint(node["style"], reinterpret_cast<unsigned int&>(style)))
				{
					auto _style_str = node["style"].value<std::string>();
					if (!_style_str)
					{
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"style\" field from font family style" << LOG.nl;
						return;
					}

					std::string style_str = *_style_str;
					if (auto s = rendering::FontStyle::from_string(style_str))
						style = *s;
					else
					{
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "\"style\" field \"" << style_str << "\" not recognized from font family style" << LOG.nl;
						return;
					}
				}

				auto _font_file = node["file"].value<std::string>();
				if (!_font_file)
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"file\" field from font family style" << LOG.nl;
					return;
				}
				ResourcePath font_file(*_font_file, file); // TODO v5 pass relative_to path when loading other files from asset loaders
				rendering::FontFamily::FontRef font;
				if (font_file.is_import_path())
				{
					auto meta = reg::MetaSplitter::meta(font_file);
					if (meta.has_type("raster_font"))
						font = context::load_raster_font(font_file);
					else
					{
						std::optional<std::string> type = meta.get_type();
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << font_file << " has unrecognized meta type: \"" << (type ? *type : "") << "\"" << LOG.nl;
						return;
					}
				}
				else
					font = context::load_font_atlas(font_file, reg::parse_uint_or(node["atlas_index"], 0));

				styles.emplace(style, std::move(font));
				});
		}

		if (toml["storage"].value<std::string>().value_or("discard") == "keep")
			internal::font_families.emplace(file, font_family);

		OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Font family [" << file << "] parsed." << LOG.nl;

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
			OLY_LOG_WARNING(true, "CONTEXT") << LOG.source_info.full_source() << "Font style (" << (unsigned int)style << ") not supported by family " << font_family << LOG.nl;
		return font;
	}

	rendering::Font load_font(const ResourcePath& file, unsigned int index)
	{
		if (file.is_import_path())
		{
			auto meta = reg::MetaSplitter::meta(file);
			if (meta.has_type("raster_font"))
				return load_raster_font(file);
			else if (meta.has_type("font_family"))
				return load_font_selection(file, rendering::FontStyle(index));
			else
			{
				std::optional<std::string> type = meta.get_type();
				OLY_LOG_ERROR(true, "CONTEXT") << LOG.source_info.full_source() << file << " has unrecognized meta type: \"" << (type ? *type : "") << "\"" << LOG.nl;
				throw Error(ErrorCode::LOAD_ASSET);
			}
		}
		else
			return load_font_atlas(file, index);
	}
}
