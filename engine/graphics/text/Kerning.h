#pragma once

#include <imp/utf.hpp>

#include <unordered_map>

namespace oly::rendering
{
    // TODO v9.3 use imp::stl_hash
	struct CodepointPairHash
	{
		size_t operator()(const std::pair<imp::utf::codepoint, imp::utf::codepoint>& p) const { return std::hash<int>{}(p.first) ^ (std::hash<int>{}(p.second) << 1); }
	};

	struct Kerning
	{
		typedef std::unordered_map<std::pair<imp::utf::codepoint, imp::utf::codepoint>, int, CodepointPairHash> Map; // maps pairs of glyphs to kerning spacing
		Map map;
	};
}
