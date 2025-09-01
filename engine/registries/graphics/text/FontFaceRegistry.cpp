#include "FontFaceRegistry.h"

#include "core/context/Context.h"
#include "core/util/Logger.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void FontFaceRegistry::clear()
	{
		font_faces.clear();
	}

	rendering::FontFaceRef FontFaceRegistry::load_font_face(const std::string& file)
	{
		auto it = font_faces.find(file);
		if (it != font_faces.end())
			return it->second;

		auto toml = load_toml(context::resource_file(file + ".oly"));
		auto node = toml["font_face"];
		if (!node.as_table())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Cannot load font face \"" << file << "\" - missing \"font_face\" table." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing font face [" << (src ? *src : "") << "]." << LOG.nl;
		}

		rendering::Kerning kerning;
		if (auto kerning_arr = node["kerning"].as_array())
		{
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
					auto dist = node["dist"].value<int64_t>();
					if (!dist)
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

					static const auto parse_codepoint = [](const std::string& sc) -> utf::Codepoint {
						if (sc.size() >= 3)
						{
							std::string prefix = sc.substr(0, 2);
							if (prefix == "U+" || prefix == "0x" || prefix == "0X" || prefix == "\\u" || prefix == "\\U" || prefix == "0h")
								return utf::Codepoint(std::stoi(sc.substr(2), nullptr, 16));
							if (sc.substr(0, 3) == "&#x" && sc.ends_with(";"))
								return utf::Codepoint(std::stoi(sc.substr(3, sc.size() - 3 - 1), nullptr, 16));
						}
						else if (sc.empty() || sc.size() == 2)
							return utf::Codepoint(0);
						return utf::Codepoint(sc[0]);
						};
					utf::Codepoint c1 = parse_codepoint(tc0->get());
					utf::Codepoint c2 = parse_codepoint(tc1->get());
					if (c1 && c2)
						kerning.map.emplace(std::make_pair(c1, c2), (int)dist.value());
					else
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In kerning #" << k_idx
												 << " - cannot parse pair codepoints: (\"" << tc0 << "\", \"" << tc1 << "\")." << LOG.nl;
				}
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse kerning #" << k_idx << " - not a TOML table." << LOG.nl;
				});
		}

		rendering::FontFaceRef font_face(context::resource_file(file).c_str(), std::move(kerning));
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			font_faces.emplace(file, font_face);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Font face [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return font_face;
	}

	void FontFaceRegistry::free_font_face(const std::string& file)
	{
		font_faces.erase(file);
	}
}
