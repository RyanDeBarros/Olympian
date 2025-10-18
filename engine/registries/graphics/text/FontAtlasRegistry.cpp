#include "FontAtlasRegistry.h"

#include "core/context/Context.h"
#include "core/context/rendering/Fonts.h"
#include "core/util/Logger.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void FontAtlasRegistry::clear()
	{
		font_atlases.clear();
	}

	rendering::FontAtlasRef FontAtlasRegistry::load_font_atlas(const std::string& file, unsigned int index)
	{
		// TODO v5 add empty filename checks to all asset loaders
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		FontAtlasKey key{ .file = file, .index = index };
		auto it = font_atlases.find(key);
		if (it != font_atlases.end())
			return it->second;

		auto toml = load_toml(context::resource_file(file + ".oly"));
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

		if (!parse_float(node["font_size"], options.font_size))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Missing \"font_size\" field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		parse_min_filter(node["min_filter"], options.min_filter);
		parse_mag_filter(node["mag_filter"], options.mag_filter);
		parse_bool(node["generate_mipmaps"], options.auto_generate_mipmaps);

		utf::String common_buffer = rendering::glyphs::COMMON;
		if (parse_bool_or(node["use_common_buffer_preset"], true))
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
			font_atlases.emplace(key, font_atlas);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Font atlas [" << (src ? *src : "") << "] at index #" << index << " parsed." << LOG.nl;
		}

		return font_atlas;
	}

	void FontAtlasRegistry::free_font_atlas(const std::string& file, unsigned int index)
	{
		font_atlases.erase({ file, index });
	}
}
