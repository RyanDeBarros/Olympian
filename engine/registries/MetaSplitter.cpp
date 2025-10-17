#include "MetaSplitter.h"

namespace oly::reg
{
    bool MetaMap::has_type(const std::string& type) const
    {
        auto it = map.find("type");
        return it != map.end() && it->second == type;
    }

    static const std::string meta_prefix = "#meta";

	MetaMap MetaSplitter::meta(const std::string& filepath)
	{
        std::ifstream file(filepath);
        MetaMap meta;

        if (!file.is_open())
            return meta;

        std::string first_line;
        if (!std::getline(file, first_line))
            return meta;

        if (!first_line.starts_with(meta_prefix.c_str()))
            return meta;

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
        }
        return meta;
	}
}
