#include "KerningSupport.h"

#include "core/util/Logger.h"
#include "registries/Loader.h"
#include "registries/graphics/text/GlyphSupport.h"

namespace oly::reg
{
	rendering::Kerning parse_kerning(TOMLNode node)
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
				if (!parse_int(node["dist"], dist))
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
}
