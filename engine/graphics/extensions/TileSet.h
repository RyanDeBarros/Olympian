#pragma once

#include "graphics/primitives/Sprites.h"
#include "core/types/SmartReference.h"

namespace oly::rendering
{
	class TileSet
	{
	public:
		struct PaintedTile
		{
			bool orthogonal[4] = { false, false, false, false }; // right, top, left, bottom
			bool diagonal[4] = { false, false, false, false }; // top-right, top-left, bottom-left, bottom-right
		};

		enum Transformation : char
		{
			NONE = 0b0,
			REFLECT_X = 0b1,
			REFLECT_Y = 0b10,
			ROTATE_90 = 0b100,
			ROTATE_180 = 0b1000,
			ROTATE_270 = 0b10000,
		};

		enum class Configuration : char
		{
			SINGLE,
			END_1,
			END_2,
			END_3,
			END_4,
			CORNER_1,
			CORNER_2,
			CORNER_3,
			CORNER_4,
			ILINE_1,
			ILINE_2,
			TBONE_1,
			TBONE_2,
			TBONE_3,
			TBONE_4,
			MIDDLE,
			CORNER_PRIME_1,
			CORNER_PRIME_2,
			CORNER_PRIME_3,
			CORNER_PRIME_4,
			TBONE_PLUS_1,
			TBONE_PLUS_2,
			TBONE_PLUS_3,
			TBONE_PLUS_4,
			TBONE_MINUS_1,
			TBONE_MINUS_2,
			TBONE_MINUS_3,
			TBONE_MINUS_4,
			TBONE_PRIME_1,
			TBONE_PRIME_2,
			TBONE_PRIME_3,
			TBONE_PRIME_4,
			MIDDLE_CORNER_1,
			MIDDLE_CORNER_2,
			MIDDLE_CORNER_3,
			MIDDLE_CORNER_4,
			MIDDLE_TBONE_1,
			MIDDLE_TBONE_2,
			MIDDLE_TBONE_3,
			MIDDLE_TBONE_4,
			MIDDLE_ACROSS_1,
			MIDDLE_ACROSS_2,
			MIDDLE_DIAGONAL_1,
			MIDDLE_DIAGONAL_2,
			MIDDLE_DIAGONAL_3,
			MIDDLE_DIAGONAL_4,
			MIDDLE_PRIME
		};

		struct Tile
		{
			size_t tex_index;
			Transformation transformation = Transformation::NONE;
		};

		struct TileDesc
		{
			std::string name;
			math::Rect2D uvs = { 0, 1, 0, 1 };

			bool operator==(const TileDesc&) const = default;
		};

		struct Assignment
		{
			TileDesc desc;
			Configuration config;
			Transformation transformation;
		};

		TileSet(const std::vector<Assignment>& assignments);

	private:
		std::vector<TileDesc> tiles;
		std::unordered_map<Configuration, Tile> assignment;

		bool valid_configuration(Configuration configuration) const;

	public:
		bool valid_6() const;
		bool valid_4x4() const;
		bool valid_4x4_2x2() const;
		TileDesc get_tile_desc(PaintedTile tile, Transformation& transformation) const;

	private:
		Tile get_assignment(Configuration config, Transformation& transformation) const;
		static Configuration get_configuration(PaintedTile tile);
	};
	typedef SmartReference<TileSet> TileSetRef;

	inline TileSet::Transformation operator&(TileSet::Transformation a, TileSet::Transformation b) { return (TileSet::Transformation)((char)a & (char)b); }
	inline TileSet::Transformation& operator&=(TileSet::Transformation& a, TileSet::Transformation b) { return a = a & b; }
	inline TileSet::Transformation operator|(TileSet::Transformation a, TileSet::Transformation b) { return (TileSet::Transformation)((char)a | (char)b); }
	inline TileSet::Transformation& operator|=(TileSet::Transformation& a, TileSet::Transformation b) { return a = a | b; }
}
