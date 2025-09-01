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
		FontAtlasKey key{ .file = file, .index = index };
		auto it = font_atlases.find(key);
		if (it != font_atlases.end())
			return it->second;

		auto toml = load_toml(context::resource_file(file + ".oly"));
		auto font_atlas_list = toml["font_atlas"].as_array();
		if (!font_atlas_list || font_atlas_list->empty())
		{
			LOG.error(true, "REG") << LOG.source_info.full_source() << "Missing or empty \"font_atlas\" array field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}
		index = glm::clamp(index, (unsigned int)0, (unsigned int)font_atlas_list->size() - 1);
		auto node = TOMLNode(*font_atlas_list->get(index));

		auto _font_size_double = node["font_size"].value<double>();
		auto _font_size_int = node["font_size"].value<int64_t>();
		if (!_font_size_double && !_font_size_int)
		{
			LOG.error(true, "REG") << LOG.source_info.full_source() << "Missing \"font_size\" field." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		rendering::FontOptions options;

		options.font_size = _font_size_double ? (float)_font_size_double.value() : (float)_font_size_int.value();
		parse_min_filter(node, "min_filter", options.min_filter);
		parse_mag_filter(node, "mag_filter", options.mag_filter);
		options.auto_generate_mipmaps = node["generate_mipmaps"].value<bool>().value_or(false);

		utf::String common_buffer = rendering::glyphs::COMMON;
		if (node["use_common_buffer_preset"].value_or<bool>(true))
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
					LOG.warning(true, "REG") << LOG.source_info.full_source() << "Unrecognized common buffer preset value \"" << common_buffer_preset << "\"." << LOG.nl;
			}
		}
		else
		{
			if (auto _common_buffer = node["common_buffer"].value<std::string>())
				common_buffer = _common_buffer.value();
		}

		rendering::FontAtlasRef font_atlas(context::load_font_face(file), options, common_buffer);
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			font_atlases.emplace(key, font_atlas);
		return font_atlas;
	}

	void FontAtlasRegistry::free_font_atlas(const std::string& file, unsigned int index)
	{
		font_atlases.erase({ file, index });
	}
}
