#pragma once

#include <imp/hash.hpp>
#include <imp/utf.hpp>

#include <unordered_map>

namespace oly::rendering
{
	struct Kerning
	{
        // maps pairs of glyphs to kerning spacing
		typedef std::unordered_map<std::pair<imp::utf::codepoint, imp::utf::codepoint>, int, imp::stl_hash<>> Map;
		
        Map map;
	};
}
