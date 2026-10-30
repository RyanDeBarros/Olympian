#include "Serializer.h"

bool imtk::serializer<oly::editor::Rect>::load(oly::editor::Rect& obj, imtk::toml_node node) const
{
    if (auto array = node.as_array())
    {
        bool fully_loaded = true;

        for (size_t i = 0; i < std::min(array->size(), oly::editor::Rect::N); ++i)
        {
            if (auto v = array->get_as<double>(i))
                obj[i] = v->get();
            else
                fully_loaded = false;
        }

        return fully_loaded;
    }
    else
        return false;
}

toml::array imtk::serializer<oly::editor::Rect>::dump(const oly::editor::Rect obj) const
{
    toml::array arr;

    arr.reserve(oly::editor::Rect::N);
    for (size_t i = 0; i < oly::editor::Rect::N; ++i)
        arr.push_back(obj[i]);

    return arr;
}

bool imtk::serializer<oly::editor::UVRect>::load(oly::editor::UVRect& obj, imtk::toml_node node) const
{
    if (auto array = node.as_array())
    {
        bool fully_loaded = true;
        
        for (size_t i = 0; i < std::min(array->size(), oly::editor::UVRect::N); ++i)
        {
            if (auto v = array->get_as<double>(i))
                obj[i] = v->get();
            else
                fully_loaded = false;
        }

        return fully_loaded;
    }
    else
        return false;
}

toml::array imtk::serializer<oly::editor::UVRect>::dump(const oly::editor::UVRect obj) const
{
    toml::array arr;

    arr.reserve(oly::editor::UVRect::N);
    for (size_t i = 0; i < oly::editor::UVRect::N; ++i)
        arr.push_back(obj[i]);

    return arr;
}

bool imtk::serializer<oly::editor::TopSidePadding>::load(oly::editor::TopSidePadding& obj, imtk::toml_node node) const
{
    if (auto array = node.as_array())
    {
        bool fully_loaded = true;
        
        for (size_t i = 0; i < std::min(array->size(), oly::editor::TopSidePadding::N); ++i)
        {
            if (auto v = array->get_as<double>(i))
                obj[i] = v->get();
            else
                fully_loaded = false;
        }

        return fully_loaded;
    }
    else
        return false;
}

toml::array imtk::serializer<oly::editor::TopSidePadding>::dump(const oly::editor::TopSidePadding obj) const
{
    toml::array arr;

    arr.reserve(oly::editor::TopSidePadding::N);
    for (size_t i = 0; i < oly::editor::TopSidePadding::N; ++i)
        arr.push_back(obj[i]);

    return arr;
}

bool imtk::serializer<oly::detail::ResourcePath>::load(oly::detail::ResourcePath& obj, imtk::toml_node node) const
{
    std::string path;
    if (imtk::serializer<std::string>{}.load(path, node))
    {
        obj = std::move(path);
        return true;
    }
    else
        return false;
}

std::string imtk::serializer<oly::detail::ResourcePath>::dump(const oly::detail::ResourcePath& obj) const
{
    return imtk::serializer<std::string>{}.dump(obj.get_resource_shorthand());
}
