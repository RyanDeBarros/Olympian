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

	inline GridCoordinate operator++(GridCoordinate& c)
	{
		c = static_cast<GridCoordinate>(static_cast<std::underlying_type_t<GridCoordinate>>(c) + 1);
		return c;
	}

	inline GridCoordinate operator++(GridCoordinate& c, int)
	{
		GridCoordinate old = c;
		++c;
		return old;
	}

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

	constexpr size_t TILE_REFLECTION_BITSET_COUNT = 2;
	extern TileReflection TILE_REFLECTION_BITSET_DEFAULT;
	extern TileReflection TILE_REFLECTION_BITSET_VALUES[TILE_REFLECTION_BITSET_COUNT];
	extern const char* TILE_REFLECTION_BITSET_NAMES[TILE_REFLECTION_BITSET_COUNT];

	inline TileReflection operator~(TileReflection a)
	{
		return static_cast<TileReflection>(~static_cast<int>(a));
	}

	inline TileReflection operator&(TileReflection a, TileReflection b)
	{
		return static_cast<TileReflection>(static_cast<int>(a) & static_cast<int>(b));
	}

	inline TileReflection& operator&=(TileReflection& a, TileReflection b)
	{
		a = a & b;
		return a;
	}

	inline TileReflection operator|(TileReflection a, TileReflection b)
	{
		return static_cast<TileReflection>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline TileReflection& operator|=(TileReflection& a, TileReflection b)
	{
		a = a | b;
		return a;
	}

	inline TileReflection operator^(TileReflection a, TileReflection b)
	{
		return static_cast<TileReflection>(static_cast<int>(a) ^ static_cast<int>(b));
	}

	inline TileReflection& operator^=(TileReflection& a, TileReflection b)
	{
		a = a ^ b;
		return a;
	}

	enum class TileRotation
	{
		None = 0,
		By90,
		By180,
		By270,
		_c
	};

	inline TileRotation operator+(TileRotation a, TileRotation b)
	{
		return static_cast<TileRotation>((static_cast<int>(a) + static_cast<int>(b)) % static_cast<int>(TileRotation::_c));
	}

	inline TileRotation& operator+=(TileRotation& a, TileRotation b)
	{
		a = a + b;
		return a;
	}

	inline TileRotation operator-(TileRotation a, TileRotation b)
	{
		return static_cast<TileRotation>((static_cast<int>(a) - static_cast<int>(b) + static_cast<int>(TileRotation::_c)) % static_cast<int>(TileRotation::_c));
	}

	inline TileRotation& operator-=(TileRotation& a, TileRotation b)
	{
		a = a - b;
		return a;
	}

	struct TileTransformation
	{
		TileReflection reflection = TileReflection::None;
		TileRotation rotation = TileRotation::None;

		TileTransformation& apply(TileTransformation trfm);
		TileTransformation& apply(TileReflection refl);
		TileTransformation& apply(TileRotation rot);
	};

	extern bool tile_config_fallback(TileConfig& config, TileTransformation& transformation, int fallback_idx);
}
