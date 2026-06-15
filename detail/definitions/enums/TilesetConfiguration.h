#pragma once

#include <array>

namespace oly::detail
{
	/*
	 * 0 1 2
	 * 7 - 3
	 * 6 5 4
	 */
	typedef unsigned char TileConfig;
	typedef std::array<std::array<bool, 3>, 3> TileConfigGrid;

	enum TileConfigIndividual : TileConfig
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

	// TODO v8 separate into two separate enums - one for Reflect and one for Rotate, since you can't combine Reflect flags or Rotate flags
	enum TileTransformation : char
	{
		None = 0,
		ReflectX = 1 << 0,
		ReflectY = 1 << 1,
		Rotate90 = 1 << 2,
		Rotate180 = 1 << 3,
		Rotate270 = 1 << 4
	};

	inline TileTransformation operator&(TileTransformation a, TileTransformation b) { return static_cast<TileTransformation>(static_cast<char>(a) & static_cast<char>(b)); }
	inline TileTransformation& operator&=(TileTransformation& a, TileTransformation b) { return a = a & b; }
	inline TileTransformation operator|(TileTransformation a, TileTransformation b) { return static_cast<TileTransformation>(static_cast<char>(a) | static_cast<char>(b)); }
	inline TileTransformation& operator|=(TileTransformation& a, TileTransformation b) { return a = a | b; }
	inline TileTransformation operator~(TileTransformation a) { return static_cast<TileTransformation>(~static_cast<char>(a)); }

	constexpr size_t TILE_TRANSFORMATION_COUNT = 5;
	extern TileTransformation TILE_TRANSFORMATION_DEFAULT;
	extern TileTransformation TILE_TRANSFORMATION_VALUES[TILE_TRANSFORMATION_COUNT];
	extern const char* TILE_TRANSFORMATION_NAMES[TILE_TRANSFORMATION_COUNT];

	extern bool tile_config_fallback(TileConfig& config, TileTransformation& transformation, int fallback_idx);
}
