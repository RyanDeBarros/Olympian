#pragma once

#include "core/Types.h"

#include "assets/ResourcePath.h"

#include <imtk.hpp>

template<>
struct imtk::serializer<oly::editor::Rect>
{
    bool load(oly::editor::Rect& obj, imtk::toml_node node) const;
    toml::array dump(const oly::editor::Rect obj) const;
};

template<>
struct imtk::serializer<oly::editor::UVRect>
{
    bool load(oly::editor::UVRect& obj, imtk::toml_node node) const;
    toml::array dump(const oly::editor::UVRect obj) const;
};

template<>
struct imtk::serializer<oly::editor::TopSidePadding>
{
    bool load(oly::editor::TopSidePadding& obj, imtk::toml_node node) const;
    toml::array dump(const oly::editor::TopSidePadding obj) const;
};

template<>
struct imtk::serializer<oly::detail::ResourcePath>
{
    bool load(oly::detail::ResourcePath& obj, imtk::toml_node node) const;
    std::string dump(const oly::detail::ResourcePath& obj) const;
};
