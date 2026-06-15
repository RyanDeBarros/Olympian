#pragma once

namespace oly::detail
{
	enum class TileConfiguration : char
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

	enum TileTransformation : char
	{
		None = 0b0,
		ReflectX = 0b1,
		ReflectY = 0b10,
		Rotate90 = 0b100,
		Rotate180 = 0b1000,
		Rotate270 = 0b10000,
	};

	inline TileTransformation operator&(TileTransformation a, TileTransformation b) { return static_cast<TileTransformation>(static_cast<char>(a) & static_cast<char>(b)); }
	inline TileTransformation& operator&=(TileTransformation& a, TileTransformation b) { return a = a & b; }
	inline TileTransformation operator|(TileTransformation a, TileTransformation b) { return static_cast<TileTransformation>(static_cast<char>(a) | static_cast<char>(b)); }
	inline TileTransformation& operator|=(TileTransformation& a, TileTransformation b) { return a = a | b; }
}
