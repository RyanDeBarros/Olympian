#include "GlyphSupport.h"

namespace oly::reg
{
	utf::Codepoint parse_codepoint(const std::string& s)
	{
		if (s.size() >= 3)
		{
			std::string prefix = s.substr(0, 2);
			if (prefix == "U+" || prefix == "0x" || prefix == "0X" || prefix == "\\u" || prefix == "\\U" || prefix == "0h")
				return utf::Codepoint(std::stoi(s.substr(2), nullptr, 16));
			else if (s.substr(0, 3) == "&#x" && s.ends_with(";"))
				return utf::Codepoint(std::stoi(s.substr(3, s.size() - 3 - 1), nullptr, 16));
			else
				return utf::Codepoint(0);
		}
		else if (s.empty() || s.size() == 2)
			return utf::Codepoint(0);
		else
			return utf::Codepoint(s[0]);
	}
}
