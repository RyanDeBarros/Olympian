#include "FontAtlasRegistry.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly
{
	namespace rendering
	{
		void FontAtlasRegistry::load(const char* font_atlas_registry_file)
		{
			auto toml = reg::load_toml(font_atlas_registry_file);
			auto font_atlas_list = toml["font_atlas"].as_array();
			if (!font_atlas_list)
				return;
			font_atlas_list->for_each([this](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto _name = node["name"].value<std::string>();
					auto _font_face = node["font face"].value<std::string>();
					auto _font_size_double = node["font size"].value<double>();
					auto _font_size_int = node["font size"].value<int64_t>();
					if (_name && _font_face && (_font_size_double || _font_size_int))
					{
						const std::string& name = _name.value();
						Constructor constructor;
						constructor.font_face_name = _font_face.value();
						constructor.options.font_size = _font_size_double ? (float)_font_size_double.value() : (float)_font_size_int.value();
						reg::parse_min_filter(node, "min filter", constructor.options.min_filter);
						reg::parse_mag_filter(node, "mag filter", constructor.options.mag_filter);
						constructor.options.auto_generate_mipmaps = node["generate mipmaps"].value<bool>().value_or(false);

						auto _common_buffer_preset = node["common buffer preset"].value<std::string>();
						if (_common_buffer_preset)
						{
							const std::string& common_buffer_preset = _common_buffer_preset.value();
							if (common_buffer_preset == "common")
								constructor.common_buffer = glyphs::COMMON;
							else if (common_buffer_preset == "alpha numeric")
								constructor.common_buffer = glyphs::ALPHA_NUMERIC;
							else if (common_buffer_preset == "numeric")
								constructor.common_buffer = glyphs::NUMERIC;
							else if (common_buffer_preset == "alphabet")
								constructor.common_buffer = glyphs::ALPHABET;
							else if (common_buffer_preset == "alphabet lowercase")
								constructor.common_buffer = glyphs::ALPHABET_LOWERCASE;
							else if (common_buffer_preset == "alphabet uppercase")
								constructor.common_buffer = glyphs::ALPHABET_UPPERCASE;
						}
						auto _common_buffer = node["common buffer"].value<std::string>();
						if (_common_buffer)
							constructor.common_buffer = _common_buffer.value();

						constructors[name] = constructor;
						if (auto _init = node["init"].value<std::string>())
						{
							auto_loaded.emplace(name, std::make_shared<FontAtlas>(create_font_atlas(name)));
							if (_init.value() == "discard")
								constructors.erase(name);
						}
					}
				}
				});
		}

		void FontAtlasRegistry::clear()
		{
			constructors.clear();
			auto_loaded.clear();
		}

		FontAtlas FontAtlasRegistry::create_font_atlas(const std::string& name) const
		{
			auto it = constructors.find(name);
			if (it == constructors.end())
				throw Error(ErrorCode::UNREGISTERED_FONT_ATLAS);
			const auto& constructor = it->second;

			return FontAtlas(context::ref_font_face(constructor.font_face_name).lock(), constructor.options, constructor.common_buffer);
		}

		std::weak_ptr<FontAtlas> FontAtlasRegistry::ref_font_atlas(const std::string& name) const
		{
			auto it = auto_loaded.find(name);
			if (it == auto_loaded.end())
				throw Error(ErrorCode::UNREGISTERED_FONT_ATLAS);
			return it->second;
		}

		void FontAtlasRegistry::delete_font_atlas(const std::string& name)
		{
			auto_loaded.erase(name);
		}
	}
}
