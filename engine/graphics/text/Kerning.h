#pragma once

#include "core/util/UTF.h"

#include <unordered_map>

namespace oly::rendering
{
	struct CodepointPairHash
	{
		size_t operator()(const std::pair<utf::Codepoint, utf::Codepoint>& p) const { return std::hash<int>{}(p.first) ^ (std::hash<int>{}(p.second) << 1); }
	};

	struct Kerning
	{
		typedef std::unordered_map<std::pair<utf::Codepoint, utf::Codepoint>, int, CodepointPairHash> Map; // maps pairs of glyphs to kerning spacing
		Map map;
	};
}
