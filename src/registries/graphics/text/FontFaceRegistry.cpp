#include "FontFaceRegistry.h"

#include "core/util/IO.h"
#include "core/base/Errors.h"
#include "core/types/Meta.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void FontFaceRegistry::load(const char* font_face_registry_file)
	{
		auto toml = load_toml(font_face_registry_file);
		auto font_face_list = toml["font_face"].as_array();
		if (!font_face_list)
			return;
		std::string root_dir = io::directory_of(font_face_registry_file);
		font_face_list->for_each([this, &root_dir](auto&& node) {
			if constexpr (toml::is_table<decltype(node)>)
			{
				auto _name = node["name"].value<std::string>();
				auto _font_file = node["file"].value<std::string>();
				if (_name && _font_file)
				{
					const std::string& name = _name.value();
					Constructor constructor;
					constructor.font_file = root_dir + _font_file.value();
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
									constructor.kerning.map.emplace(std::make_pair(c1, c2), k);
							}
						}
					}
					if (auto _init = node["init"].value<std::string>())
					{
						if (_init.value() == "discard")
							auto_loaded.emplace(name, std::make_shared<rendering::FontFace>(constructor.font_file.c_str(), std::move(constructor.kerning)));
						else
						{
							auto_loaded.emplace(name, std::make_shared<rendering::FontFace>(constructor.font_file.c_str(), dupl(constructor.kerning)));
							constructors[name] = constructor;
						}
					}
					else
						constructors[name] = constructor;
				}
			}
			});
	}

	void FontFaceRegistry::clear()
	{
		constructors.clear();
		auto_loaded.clear();
	}

	rendering::FontFace FontFaceRegistry::create_font_face(const std::string& name) const
	{
		auto it = constructors.find(name);
		if (it == constructors.end())
			throw Error(ErrorCode::UNREGISTERED_FONT_FACE);
		const auto& constructor = it->second;
		return rendering::FontFace(constructor.font_file.c_str(), dupl(constructor.kerning));
	}

	std::weak_ptr<rendering::FontFace> FontFaceRegistry::ref_font_face(const std::string& name) const
	{
		auto it = auto_loaded.find(name);
		if (it == auto_loaded.end())
			throw Error(ErrorCode::UNREGISTERED_FONT_FACE);
		return it->second;
	}

	void FontFaceRegistry::delete_font_face(const std::string& name)
	{
		auto_loaded.erase(name);
	}
}
