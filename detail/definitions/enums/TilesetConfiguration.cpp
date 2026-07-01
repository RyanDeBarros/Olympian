#include "TilesetConfiguration.h"

#include <utility>

namespace oly::detail
{
	TileConfig tile_config_from_grid(const TileConfigGrid grid)
	{
		TileConfig config = 0;
		
		if (grid[GridCoordinate::Top][GridCoordinate::Left])
			config |= TileConfigIndividual::TopLeftCorner;

		if (grid[GridCoordinate::Top][GridCoordinate::Middle])
			config |= TileConfigIndividual::TopEdge;

		if (grid[GridCoordinate::Top][GridCoordinate::Right])
			config |= TileConfigIndividual::TopRightCorner;

		if (grid[GridCoordinate::Middle][GridCoordinate::Right])
			config |= TileConfigIndividual::RightEdge;

		if (grid[GridCoordinate::Bottom][GridCoordinate::Right])
			config |= TileConfigIndividual::BottomRightCorner;

		if (grid[GridCoordinate::Bottom][GridCoordinate::Middle])
			config |= TileConfigIndividual::BottomEdge;

		if (grid[GridCoordinate::Bottom][GridCoordinate::Left])
			config |= TileConfigIndividual::BottomLeftCorner;

		if (grid[GridCoordinate::Middle][GridCoordinate::Left])
			config |= TileConfigIndividual::LeftEdge;

		return config;
	}

	TileConfigGrid grid_from_tile_config(TileConfig config)
	{
		TileConfigGrid grid;
		grid[GridCoordinate::Top][GridCoordinate::Left] = config & TileConfigIndividual::TopLeftCorner;
		grid[GridCoordinate::Top][GridCoordinate::Middle] = config & TileConfigIndividual::TopEdge;
		grid[GridCoordinate::Top][GridCoordinate::Right] = config & TileConfigIndividual::TopRightCorner;
		grid[GridCoordinate::Middle][GridCoordinate::Right] = config & TileConfigIndividual::RightEdge;
		grid[GridCoordinate::Bottom][GridCoordinate::Right] = config & TileConfigIndividual::BottomRightCorner;
		grid[GridCoordinate::Bottom][GridCoordinate::Middle] = config & TileConfigIndividual::BottomEdge;
		grid[GridCoordinate::Bottom][GridCoordinate::Left] = config & TileConfigIndividual::BottomLeftCorner;
		grid[GridCoordinate::Middle][GridCoordinate::Left] = config & TileConfigIndividual::LeftEdge;
		return grid;
	}

	TileConfig rotate_tile_config(TileConfig config, int quarter_turns)
	{
		if (quarter_turns > 0)
		{
			for (int i = 0; i < quarter_turns; ++i)
			{
				unsigned char mov = 0b1100'0000 & config;
				config <<= 2;
				config |= (mov >> 6);
			}
		}
		else if (quarter_turns < 0)
		{
			for (int i = 0; i > quarter_turns; --i)
			{
				unsigned char mov = 0b0000'0011 & config;
				config >>= 2;
				config |= (mov << 6);
			}
		}

		return config;
	}

	TileConfig reflect_x_tile_config(TileConfig config)
	{
		TileConfigGrid grid = grid_from_tile_config(config);
		for (int i = GridCoordinate::Top; i <= GridCoordinate::Bottom; ++i)
			std::swap(grid[i][GridCoordinate::Left], grid[i][GridCoordinate::Right]);
		return tile_config_from_grid(grid);
	}

	TileConfig reflect_y_tile_config(TileConfig config)
	{
		TileConfigGrid grid = grid_from_tile_config(config);
		for (int i = GridCoordinate::Left; i <= GridCoordinate::Right; ++i)
			std::swap(grid[GridCoordinate::Top][i], grid[GridCoordinate::Bottom][i]);
		return tile_config_from_grid(grid);
	}

	bool tile_config_is_available(GridCoordinate x, GridCoordinate y, const TileConfigGrid grid)
	{
		if (x == GridCoordinate::Left && y == GridCoordinate::Top)
			return grid[GridCoordinate::Top][GridCoordinate::Middle] && grid[GridCoordinate::Middle][GridCoordinate::Left];
		else if (x == GridCoordinate::Right && y == GridCoordinate::Top)
			return grid[GridCoordinate::Top][GridCoordinate::Middle] && grid[GridCoordinate::Middle][GridCoordinate::Right];
		else if (x == GridCoordinate::Left && y == GridCoordinate::Bottom)
			return grid[GridCoordinate::Bottom][GridCoordinate::Middle] && grid[GridCoordinate::Middle][GridCoordinate::Left];
		else if (x == GridCoordinate::Right && y == GridCoordinate::Bottom)
			return grid[GridCoordinate::Bottom][GridCoordinate::Middle] && grid[GridCoordinate::Middle][GridCoordinate::Right];
		else
			return true;
	}

	std::ostream& operator<<(std::ostream& os, TileReflection reflection)
	{
		os << "TileReflection(";

		switch (reflection)
		{
		case TileReflection::None:
			os << "None";
			break;

		case TileReflection::X:
			os << "X";
			break;

		case TileReflection::Y:
			os << "Y";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}

	TileReflection TILE_REFLECTION_BITSET_DEFAULT = TileReflection::None;

	TileReflection TILE_REFLECTION_BITSET_VALUES[TILE_REFLECTION_BITSET_COUNT] = {
		TileReflection::X,
		TileReflection::Y
	};

	const char* TILE_REFLECTION_BITSET_NAMES[TILE_REFLECTION_BITSET_COUNT] = {
		"X",
		"Y"
	};

	std::ostream& operator<<(std::ostream& os, TileRotation rotation)
	{
		os << "TileRotation(";

		switch (rotation)
		{
		case TileRotation::None:
			os << "None";
			break;

		case TileRotation::By90:
			os << "By90";
			break;

		case TileRotation::By180:
			os << "By180";
			break;

		case TileRotation::By270:
			os << "By270";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}

	static TileConfig tile_config_toggle_fillable_corner(TileConfig config, TileConfigIndividual corner)
	{
		switch (corner)
		{
		case TileConfigIndividual::TopRightCorner:
			if (config & TileConfigIndividual::TopEdge && config & TileConfigIndividual::RightEdge)
				config ^= corner;
			break;

		case TileConfigIndividual::TopLeftCorner:
			if (config & TileConfigIndividual::TopEdge && config & TileConfigIndividual::LeftEdge)
				config ^= corner;
			break;

		case TileConfigIndividual::BottomLeftCorner:
			if (config & TileConfigIndividual::BottomEdge && config & TileConfigIndividual::LeftEdge)
				config ^= corner;
			break;

		case TileConfigIndividual::BottomRightCorner:
			if (config & TileConfigIndividual::BottomEdge && config & TileConfigIndividual::RightEdge)
				config ^= corner;
			break;
		}

		return config;
	}

	static TileConfig tile_config_toggle_fillable_corners(TileConfig config)
	{
		config = tile_config_toggle_fillable_corner(config, TileConfigIndividual::TopRightCorner);
		config = tile_config_toggle_fillable_corner(config, TileConfigIndividual::TopLeftCorner);
		config = tile_config_toggle_fillable_corner(config, TileConfigIndividual::BottomLeftCorner);
		config = tile_config_toggle_fillable_corner(config, TileConfigIndividual::BottomRightCorner);
		return config;
	}

	static TileConfig filled_tile_config(TileConfig config)
	{
		if (config & TileConfigIndividual::TopEdge && config & TileConfigIndividual::RightEdge)
			config |= TileConfigIndividual::TopRightCorner;

		if (config & TileConfigIndividual::TopEdge && config & TileConfigIndividual::LeftEdge)
			config |= TileConfigIndividual::TopLeftCorner;

		if (config & TileConfigIndividual::BottomEdge && config & TileConfigIndividual::LeftEdge)
			config |= TileConfigIndividual::BottomLeftCorner;

		if (config & TileConfigIndividual::BottomEdge && config & TileConfigIndividual::RightEdge)
			config |= TileConfigIndividual::BottomRightCorner;

		return config;
	}

	static bool tile_config_fallback_single_rotation(TileConfig& config, TileTransformation& transformation, int fallback_idx)
	{
		if (fallback_idx == 0)
		{
			config = rotate_tile_config(config, 1);
			transformation.apply(TileRotation::By90);
			return true;
		}
		else
			return false;
	}

	static bool tile_config_fallback_rotations(TileConfig& config, TileTransformation& transformation, int fallback_idx)
	{
		if (fallback_idx == 0)
		{
			config = rotate_tile_config(config, 2);
			transformation.apply(TileRotation::By180);
			return true;
		}
		else if (fallback_idx == 1)
		{
			config = rotate_tile_config(config, 1);
			transformation.apply(TileRotation::By90);
			return true;
		}
		else if (fallback_idx == 2)
		{
			config = rotate_tile_config(config, -1);
			transformation.apply(TileRotation::By270);
			return true;
		}
		else
			return false;
	}

	static bool tile_config_fallback_filled(TileConfig& config, TileTransformation& transformation, int fallback_idx)
	{
		if (fallback_idx == 0)
		{
			config = filled_tile_config(config);
			return true;
		}

		TileConfig c = filled_tile_config(config);
		if (tile_config_fallback_rotations(c, transformation, fallback_idx - 1))
		{
			config = c;
			return true;
		}
		else
			return false;
	}

	static bool tile_config_fallback_toggle_fillable_corners(TileConfig& config, TileTransformation& transformation, int fallback_idx)
	{
		if (fallback_idx == 0)
		{
			config = tile_config_toggle_fillable_corners(config);
			return true;
		}

		TileConfig c = tile_config_toggle_fillable_corners(config);
		if (tile_config_fallback_rotations(c, transformation, fallback_idx - 1))
		{
			config = c;
			return true;
		}
		else
			return false;
	}

	bool simplify_tile_config(TileConfig& config)
	{
		bool simplified = false;

		if (config & TileConfigIndividual::TopRightCorner && !(config & TileConfigIndividual::TopEdge && config & TileConfigIndividual::RightEdge))
		{
			config &= ~TileConfigIndividual::TopRightCorner;
			simplified = true;
		}

		if (config & TileConfigIndividual::TopLeftCorner && !(config & TileConfigIndividual::TopEdge && config & TileConfigIndividual::LeftEdge))
		{
			config &= ~TileConfigIndividual::TopLeftCorner;
			simplified = true;
		}

		if (config & TileConfigIndividual::BottomLeftCorner && !(config & TileConfigIndividual::BottomEdge && config & TileConfigIndividual::LeftEdge))
		{
			config &= ~TileConfigIndividual::BottomLeftCorner;
			simplified = true;
		}

		if (config & TileConfigIndividual::BottomRightCorner && !(config & TileConfigIndividual::BottomEdge && config & TileConfigIndividual::RightEdge))
		{
			config &= ~TileConfigIndividual::BottomRightCorner;
			simplified = true;
		}

		return simplified;
	}

	bool tile_config_fallback(TileConfig& config, TileTransformation& transformation, int fallback_idx)
	{
		switch (config)
		{
		case TileConfigIndividual::Single:
		case TileConfigIndividual::Full:
			return false;

		case TileConfigIndividual::AllEdges:
			if (fallback_idx == 0)
			{
				config = TileConfigIndividual::Full;
				return true;
			}
			else
				return false;

			// ================================================================================ LINES ================================================================================
		case TileConfigIndividual::LeftEdge | TileConfigIndividual::RightEdge:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::BottomEdge:
			return tile_config_fallback_single_rotation(config, transformation, fallback_idx);

			// ================================================================================ ENDS ================================================================================
		case TileConfigIndividual::RightEdge:
		case TileConfigIndividual::LeftEdge:
		case TileConfigIndividual::TopEdge:
		case TileConfigIndividual::BottomEdge:
			// ================================================================================ CORNERS ================================================================================
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::TopRightCorner:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::TopLeftCorner:
		case TileConfigIndividual::BottomEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::BottomLeftCorner:
		case TileConfigIndividual::BottomEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::BottomRightCorner:
			// ================================================================================ T-BONES ================================================================================
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::BottomEdge | TileConfigIndividual::TopRightCorner | TileConfigIndividual::BottomRightCorner:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::TopLeftCorner | TileConfigIndividual::TopRightCorner:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::BottomEdge | TileConfigIndividual::BottomLeftCorner | TileConfigIndividual::TopLeftCorner:
		case TileConfigIndividual::RightEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::BottomEdge | TileConfigIndividual::BottomRightCorner | TileConfigIndividual::BottomLeftCorner:
			return tile_config_fallback_rotations(config, transformation, fallback_idx);

			// ================================================================================ CORNERS ================================================================================
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::LeftEdge:
		case TileConfigIndividual::BottomEdge | TileConfigIndividual::LeftEdge:
		case TileConfigIndividual::BottomEdge | TileConfigIndividual::RightEdge:
			// ================================================================================ T-BONES ================================================================================
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::BottomEdge:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::LeftEdge:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::BottomEdge:
		case TileConfigIndividual::BottomEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::LeftEdge:
			// ================================================================================ MIDDLING ================================================================================
		case TileConfigIndividual::AllEdges | TileConfigIndividual::TopLeftCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::TopRightCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::BottomRightCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::BottomLeftCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::TopLeftCorner | TileConfigIndividual::TopRightCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::TopRightCorner | TileConfigIndividual::BottomRightCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::BottomRightCorner | TileConfigIndividual::BottomLeftCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::BottomLeftCorner | TileConfigIndividual::TopLeftCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::TopLeftCorner | TileConfigIndividual::TopRightCorner | TileConfigIndividual::BottomRightCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::TopRightCorner | TileConfigIndividual::BottomRightCorner | TileConfigIndividual::BottomLeftCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::BottomRightCorner | TileConfigIndividual::BottomLeftCorner | TileConfigIndividual::TopLeftCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::BottomLeftCorner | TileConfigIndividual::TopLeftCorner | TileConfigIndividual::TopRightCorner:
			return tile_config_fallback_rotations(config, transformation, fallback_idx)
				|| tile_config_fallback_filled(config, transformation, fallback_idx - 3);

			// ================================================================================ T-BONES ================================================================================
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::BottomEdge | TileConfigIndividual::TopRightCorner:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::BottomEdge | TileConfigIndividual::BottomRightCorner:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::TopLeftCorner:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::RightEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::TopRightCorner:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::BottomEdge | TileConfigIndividual::BottomLeftCorner:
		case TileConfigIndividual::TopEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::BottomEdge | TileConfigIndividual::TopLeftCorner:
		case TileConfigIndividual::RightEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::BottomEdge | TileConfigIndividual::BottomRightCorner:
		case TileConfigIndividual::RightEdge | TileConfigIndividual::LeftEdge | TileConfigIndividual::BottomEdge | TileConfigIndividual::BottomLeftCorner:
			return tile_config_fallback_rotations(config, transformation, fallback_idx)
				|| tile_config_fallback_toggle_fillable_corners(config, transformation, fallback_idx - 3)
				|| tile_config_fallback_filled(config, transformation, fallback_idx - 3 - 4);

			// ================================================================================ MIDDLING ================================================================================
		case TileConfigIndividual::AllEdges | TileConfigIndividual::TopLeftCorner | TileConfigIndividual::BottomRightCorner:
		case TileConfigIndividual::AllEdges | TileConfigIndividual::TopRightCorner | TileConfigIndividual::BottomLeftCorner:
			return tile_config_fallback_single_rotation(config, transformation, fallback_idx)
				|| tile_config_fallback_filled(config, transformation, fallback_idx - 1);

		default:
			if (simplify_tile_config(config))
			{
				if (fallback_idx == 0)
					return true;
				else
					return tile_config_fallback(config, transformation, fallback_idx - 1);
			}
			else
				return false;
		}
	}

	TileTransformation& TileTransformation::apply(TileTransformation trfm)
	{
		return apply(trfm.reflection).apply(trfm.rotation);
	}

	TileTransformation& TileTransformation::apply(TileReflection refl)
	{
		reflection ^= refl;
		return *this;
	}

	TileTransformation& TileTransformation::apply(TileRotation rot)
	{
		if (static_cast<bool>(reflection & TileReflection::X) == static_cast<bool>(reflection & TileReflection::Y))
		{
			rotation += rot;
		}
		else
		{
			rotation -= rot;
			reflection ^= TileReflection::X;
		}
		return *this;
	}
}
