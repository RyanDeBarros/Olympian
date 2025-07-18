#include "FontAtlasRegistry.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void FontAtlasRegistry::clear()
	{
		font_atlases.clear();
	}

	rendering::FontAtlasRef FontAtlasRegistry::load_font_atlas(const std::string& file, unsigned int index)
	{
		auto it = font_atlases.find(file);
		if (it != font_atlases.end())
			return it->second;

		auto toml = load_toml(context::context_filepath() + file + ".oly");
		auto font_atlas_list = toml["font_atlas"].as_array();
		if (font_atlas_list->empty())
			throw Error(ErrorCode::LOAD_ASSET);
		index = glm::clamp(index, (unsigned int)0, (unsigned int)font_atlas_list->size() - 1);
		auto node = TOMLNode(*font_atlas_list->get(index));

		auto _font_size_double = node["font size"].value<double>();
		auto _font_size_int = node["font size"].value<int64_t>();
		if (!_font_size_double && !_font_size_int)
			throw Error(ErrorCode::LOAD_ASSET);

		rendering::FontOptions options;

		options.font_size = _font_size_double ? (float)_font_size_double.value() : (float)_font_size_int.value();
		parse_min_filter(node, "min filter", options.min_filter);
		parse_mag_filter(node, "mag filter", options.mag_filter);
		options.auto_generate_mipmaps = node["generate mipmaps"].value<bool>().value_or(false);

		utf::String common_buffer = rendering::glyphs::COMMON;
		if (auto _common_buffer_preset = node["common buffer preset"].value<std::string>())
		{
			const std::string& common_buffer_preset = _common_buffer_preset.value();
			if (common_buffer_preset == "common")
				; // already initialized to common
			else if (common_buffer_preset == "alpha numeric")
				common_buffer = rendering::glyphs::ALPHA_NUMERIC;
			else if (common_buffer_preset == "numeric")
				common_buffer = rendering::glyphs::NUMERIC;
			else if (common_buffer_preset == "alphabet")
				common_buffer = rendering::glyphs::ALPHABET;
			else if (common_buffer_preset == "alphabet lowercase")
				common_buffer = rendering::glyphs::ALPHABET_LOWERCASE;
			else if (common_buffer_preset == "alphabet uppercase")
				common_buffer = rendering::glyphs::ALPHABET_UPPERCASE;
		}
		else if (auto _common_buffer = node["common buffer"].value<std::string>())
			common_buffer = _common_buffer.value();

		rendering::FontAtlasRef font_atlas(context::load_font_face(file), options, common_buffer);
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			font_atlases.emplace(file, font_atlas);
		return font_atlas;
	}

	void FontAtlasRegistry::free_font_atlas(const std::string& file)
	{
		font_atlases.erase(file);
	}
}
