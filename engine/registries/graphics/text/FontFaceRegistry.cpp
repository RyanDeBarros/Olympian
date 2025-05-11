#include "FontFaceRegistry.h"

#include "core/base/Context.h"

namespace oly::reg
{
	void FontFaceRegistry::clear()
	{
		font_faces.clear();
	}

	rendering::FontFaceRes FontFaceRegistry::load_font_face(const std::string& file)
	{
		auto it = font_faces.find(file);
		if (it != font_faces.end())
			return it->second;

		auto toml = load_toml(context::context_filepath() + file + ".oly");
		auto node = toml["font_face"];

		rendering::Kerning kerning;
		if (auto kerning_arr = node["kerning"].as_array())
		{
			for (const auto& node : *kerning_arr)
			{
				if (auto triplet = node.as_array())
				{
					utf::Codepoint c1, c2;
					int k;
					static const auto parse_codepoint = [](const std::string& sc) -> utf::Codepoint {
						if (sc.size() >= 3)
						{
							std::string prefix = sc.substr(0, 2);
							if (prefix == "U+" || prefix == "0x" || prefix == "0X" || prefix == "\\u" || prefix == "\\U" || prefix == "0h")
								return utf::Codepoint(std::stoi(sc.substr(2), nullptr, 16));
							if (sc.substr(0, 3) == "&#x" && sc.ends_with(";"))
								return utf::Codepoint(std::stoi(sc.substr(3, sc.size() - 3 - 1), nullptr, 16));
						}
						else if (sc.empty())
							return utf::Codepoint(0);
						return utf::Codepoint(sc[0]);
						};
					if (auto tc1 = triplet->get_as<std::string>(0))
						c1 = parse_codepoint(tc1->get());
					else
						continue;
					if (auto tc2 = triplet->get_as<std::string>(1))
						c2 = parse_codepoint(tc2->get());
					else
						continue;
					if (auto tk = triplet->get_as<int64_t>(2))
						k = (int)tk->get();
					else
						continue;
					if (c1 && c2)
						kerning.map.emplace(std::make_pair(c1, c2), k);
				}
			}
		}

		rendering::FontFaceRes font_face = std::make_shared<rendering::FontFace>((context::context_filepath() + file).c_str(), std::move(kerning));
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			font_faces.emplace(file, font_face);
		return font_face;
	}

	void FontFaceRegistry::free_font_face(const std::string& file)
	{
		font_faces.erase(file);
	}
}
