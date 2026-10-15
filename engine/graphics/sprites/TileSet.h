#pragma once

#include "core/base/SmartReference.h"

#include "core/math/Shapes.h"

#include "assets/ResourcePath.h"

#include "definitions/enums/TilesetConfiguration.h"

#include <unordered_set>

namespace oly::rendering
{
    class TileSet
    {
    public:
        struct TileDesc
        {
            detail::ResourcePath file;
            math::UVRect uvs;
            unsigned int file_index = 0;

            bool operator==(const TileDesc&) const = default;
        };

        struct Assignment
        {
            TileDesc desc;
            detail::TileConfig config = 0;
            detail::TileTransformation transformation;
        };

    private:
        struct Tile
        {
            size_t tex_index;
            detail::TileTransformation transformation;
        };

    private:
        std::vector<TileDesc> tiles_;
        std::unordered_map<detail::TileConfig, Tile> assignments_;

    public:
        TileSet(const std::vector<Assignment>& assignments = {});

    public:
        static TileSet load(TOMLNode node);
        void overload(TOMLNode node);

        Assignment get_tile_assignment(const detail::TileConfigGrid tile) const;

    private:
        void load_assignments(const std::vector<Assignment>& assignments);
        std::optional<Tile> get_assignment(
            detail::TileConfig config,
            detail::TileTransformation& transformation,
            std::unordered_set<detail::TileConfig>& fallbacks_seen) const;
    };

    using TileSetRef = SmartReference<TileSet>;
}
