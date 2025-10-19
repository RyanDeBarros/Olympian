#include "RasterFontRegistry.h"

#include "core/context/Context.h"
#include "core/context/rendering/Textures.h"
#include "core/util/LoggerOperators.h"
#include "registries/Loader.h"
#include "registries/MetaSplitter.h"
#include "registries/graphics/text/KerningSupport.h"
#include "registries/graphics/text/GlyphSupport.h"

namespace oly::reg
{
	void RasterFontRegistry::clear()
	{
		raster_fonts.clear();
	}

	rendering::RasterFontRef RasterFontRegistry::load_raster_font(const ResourcePath& file)
	{
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto it = raster_fonts.find(file);
		if (it != raster_fonts.end())
			return it->second;

		OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing raster font [" << file << "]..." << LOG.nl;

		auto meta = MetaSplitter::meta(file); // TODO v5 use MetaSplitter throughout registries
		if (!meta.has_type("raster_font"))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Meta fields do not contain raster font type." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto table = load_toml(file);
		TOMLNode toml = (TOMLNode)table;

		float space_advance_width;
		if (!parse_float(toml["space_advance_width"], space_advance_width))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing \"space_advance_width\" field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		float line_height;
		if (!parse_float(toml["line_height"], line_height))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing \"line_height\" field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		glm::vec2 font_scale = glm::vec2(1.0f);
		if (auto a = toml["font_scale"])
		{
			if (!parse_vec(a, font_scale))
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
				unsigned int tidx = parse_uint_or(g["texture_file"], 0);
				if (tidx < texture_files.size())
					texture_file = texture_files[tidx];
				else
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Texture file indexer (" << tidx
												 << ") is out of range (" << texture_files.size() << "), skipping glyph..." << LOG.nl;
					return;
				}

				unsigned int texture_index = parse_uint_or(g["texture_index"], 0);

				math::IRect2D location;
				if (!parse_shape(g["location"], location))
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"location\" field, skipping glyph..." << LOG.nl;
					return;
				}

				math::TopSidePadding padding = parse_topside_padding(g["padding"]);

				math::PositioningMode origin_offset_mode = math::PositioningMode::RELATIVE;
				parse_enum(g["origin_offset_mode"], origin_offset_mode);

				glm::vec2 origin_offset = {};
				parse_vec(g["origin_offset"], origin_offset);

				glyphs.emplace(codepoint, rendering::RasterFontGlyph(context::load_texture(texture_file, texture_index), location, padding, origin_offset_mode, origin_offset));
			});
		}

		rendering::RasterFontRef raster_font(std::move(glyphs), space_advance_width, line_height, font_scale, reg::parse_kerning(toml));
		if (toml["storage"].value<std::string>().value_or("discard") == "keep")
			raster_fonts.emplace(file, raster_font);

		OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Raster font [" << file << "] parsed." << LOG.nl;

		return raster_font;
	}

	void RasterFontRegistry::free_raster_font(const ResourcePath& file)
	{
		raster_fonts.erase(file);
	}
}
