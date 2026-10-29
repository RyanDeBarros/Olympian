#pragma once

#include <imp/bitmask.hpp>

#include <array>
#include <ostream>

namespace oly::detail
{
    /*
     * 0 1 2
     * 7 - 3
     * 6 5 4
     */
    using TileConfig = unsigned char;
    using TileConfigGrid = std::array<std::array<bool, 3>, 3>;

    enum TileConfigIndividual
        : TileConfig
    {
        Single = 0,
        TopLeftCorner = 1 << 0,
        TopEdge = 1 << 1,
        TopRightCorner = 1 << 2,
        RightEdge = 1 << 3,
        BottomRightCorner = 1 << 4,
        BottomEdge = 1 << 5,
        BottomLeftCorner = 1 << 6,
        LeftEdge = 1 << 7,

        // Special
        AllEdges = TopEdge | RightEdge | BottomEdge | LeftEdge,
        Full = 0xFF
    };

    enum GridCoordinate
    {
        Top = 0,
        Left = 0,
        Middle = 1,
        Bottom = 2,
        Right = 2
    };

    extern GridCoordinate operator++(GridCoordinate& c);
    extern GridCoordinate operator++(GridCoordinate& c, int);

    /*
     * grid[0][0] grid[0][1] grid[0][2]
     * grid[1][0]     --     grid[1][2]
     * grid[2][0] grid[2][1] grid[2][2]
     */
    extern TileConfig tile_config_from_grid(const TileConfigGrid grid);
    extern TileConfigGrid grid_from_tile_config(TileConfig config);
    extern TileConfig rotate_tile_config(TileConfig config, int quarter_turns);
    extern TileConfig reflect_x_tile_config(TileConfig config);
    extern TileConfig reflect_y_tile_config(TileConfig config);
    extern bool tile_config_is_available(GridCoordinate x, GridCoordinate y, const TileConfigGrid grid);

    enum class TileReflection
    {
        None = 0,
        X = 1 << 0,
        Y = 1 << 1
    };

    extern std::ostream& operator<<(std::ostream& os, TileReflection reflection);

    constexpr size_t kTileReflectionBitsetCount = 2;
    extern const TileReflection kTileReflectionBitsetDefault;
    extern const TileReflection kTileReflectionBitsetValues[kTileReflectionBitsetCount];
    extern const char* const kTileReflectionBitsetNames[kTileReflectionBitsetCount];

    IMP_IMPLEMENT_BITMASK(TileReflection);

    enum class TileRotation
    {
        None = 0,
        By90,
        By180,
        By270,
        _c
    };

    extern std::ostream& operator<<(std::ostream& os, TileRotation rotation);

    extern TileRotation operator+(TileRotation a, TileRotation b);
    extern TileRotation& operator+=(TileRotation& a, TileRotation b);
    extern TileRotation operator-(TileRotation a, TileRotation b);
    extern TileRotation& operator-=(TileRotation& a, TileRotation b);

    struct TileTransformation
    {
        TileReflection reflection = TileReflection::None;
        TileRotation rotation = TileRotation::None;

        TileTransformation& apply(TileTransformation trfm);
        TileTransformation& apply(TileReflection refl);
        TileTransformation& apply(TileRotation rot);
    };

    extern bool simplify_tile_config(TileConfig& config);
    extern bool tile_config_fallback(TileConfig& config, TileTransformation& transformation, int fallback_idx);
}
