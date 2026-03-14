#pragma once

#include "core/types/SmartReference.h"
#include "core/math/Shapes.h"
#include "core/util/ResourcePath.h"

#include <unordered_set>

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
			None = 0b0,
			ReflectX = 0b1,
			ReflectY = 0b10,
			Rotate90 = 0b100,
			Rotate180 = 0b1000,
			Rotate270 = 0b10000,
		};

		enum class Configuration : char
		{
			Single,
			End1,
			End2,
			End3,
			End4,
			Corner1,
			Corner2,
			Corner3,
			Corner4,
			ILine1,
			ILine2,
			TBone1,
			TBone2,
			TBone3,
			TBone4,
			Middle,
			CornerPrime1,
			CornerPrime2,
			CornerPrime3,
			CornerPrime4,
			TBonePlus1,
			TBonePlus2,
			TBonePlus3,
			TBonePlus4,
			TBoneMinus1,
			TBoneMinus2,
			TBoneMinus3,
			TBoneMinus4,
			TBonePrime1,
			TBonePrime2,
			TBonePrime3,
			TBonePrime4,
			MiddleCorner1,
			MiddleCorner2,
			MiddleCorner3,
			MiddleCorner4,
			MiddleTBone1,
			MiddleTBone2,
			MiddleTBone3,
			MiddleTBone4,
			MiddleAcross1,
			MiddleAcross2,
			MiddleDiagonal1,
			MiddleDiagonal2,
			MiddleDiagonal3,
			MiddleDiagonal4,
			MiddlePrime,
			_c
		};

		struct Tile
		{
			size_t tex_index;
			Transformation transformation = Transformation::None;
		};

		struct TileDesc
		{
			ResourcePath file;
			math::UVRect uvs;

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
