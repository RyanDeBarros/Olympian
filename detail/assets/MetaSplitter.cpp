#include "MetaSplitter.h"

#include "assets/ResourcePath.h"
#include "assets/TranslateKey.h"
#include "definitions/Keys.h"

#include <fstream>
#include <sstream>

namespace oly::detail
{
    static const std::string meta_prefix = "#meta";

    std::optional<Key> MetaMap::get_type() const
    {
        auto it = map.find(Key::Meta_Type);
        return it != map.end() ? std::make_optional(decode_key(it->second)) : std::nullopt;
    }

    bool MetaMap::has_type(Key type) const
    {
        auto it = map.find(Key::Meta_Type);
        return it != map.end() && decode_key(it->second) == type;
    }

    std::string MetaMap::get_version() const
    {
        auto it = map.find(Key::Meta_Version);
        return it != map.end() ? it->second : "";
    }

	MetaMap MetaSplitter::decode_meta(const ResourcePath& filepath)
	{
        std::ifstream file = filepath.get_ifstream();
        if (!file.is_open())
            return {};

        std::string first_line;
        if (!std::getline(file, first_line))
            return {};

        if (!first_line.starts_with(meta_prefix.c_str()))
            return {};

        MetaMap meta;
        std::istringstream iss(first_line.substr(meta_prefix.size()));
        std::string token;
        while (iss >> token)
        {
            auto pos = token.find('=');
            if (pos != std::string::npos)
            {
                std::string key = token.substr(0, pos);
                std::string value = token.substr(pos + 1);

                if (!value.empty() && value.front() == '"' && value.back() == '"')
                {
                    value.pop_back();
                    value.erase(value.begin());
                }
                meta.map[decode_key(key)] = value;
            }
            else
                meta.map[decode_key(token)] = encode_key(Key::Meta_Exists);
        }
        return meta;
	}

    std::string MetaSplitter::encode_meta(const MetaMap& meta)
    {
        std::ostringstream oss;
        oss << meta_prefix << ' ';

        for (const auto& [key, value] : meta.map)
            oss << encode_key(key) << "=\"" << value << "\" ";

        oss << '\n';
        return oss.str();
    }
}
