#include "MetaSplitter.h"

#include <fstream>
#include <sstream>

namespace oly::detail
{
    std::optional<std::string> MetaMap::get_type() const
    {
        auto it = map.find("type");
        if (it != map.end())
            return it->second;
        else
            return std::nullopt;
    }

    bool MetaMap::has_type(const std::string_view type) const
    {
        auto it = map.find("type");
        return it != map.end() && it->second == type;
    }

    static const std::string meta_prefix = "#meta";

	MetaMap MetaSplitter::meta(const std::filesystem::path& filepath)
	{
        std::ifstream file(filepath);
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
                meta.map[key] = value;
            }
            else
                meta.map[token] = "1";
        }
        return meta;
	}
}
