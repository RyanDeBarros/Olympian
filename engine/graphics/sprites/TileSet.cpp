#include "TileSet.h"

namespace oly::rendering
{
	TileSet::TileSet(const std::vector<Assignment>& assignments)
	{
		for (const Assignment& a : assignments)
		{
			auto texture_it = std::find(tiles.begin(), tiles.end(), a.desc);
			Tile tile{ .tex_index = size_t(texture_it - tiles.begin()) };
			if (texture_it == tiles.end())
				tiles.push_back(a.desc);
			tile.transformation &= a.transformation;
			assignment[a.config] = tile;
		}
	}

	bool TileSet::valid_configuration(Configuration configuration) const
	{
		auto it = assignment.find(configuration);
		if (it == assignment.end())
			return false;
		else
			return it->second.tex_index < tiles.size();
	}

	bool TileSet::valid_6() const
	{
		return  valid_configuration(Configuration::Single) &&   valid_configuration(Configuration::End1) && valid_configuration(Configuration::CornerPrime1)
			&& valid_configuration(Configuration::ILine1) && valid_configuration(Configuration::TBonePrime1) && valid_configuration(Configuration::MiddlePrime);
	}

	bool TileSet::valid_4x4() const
	{
		return         valid_configuration(Configuration::Single) &&          valid_configuration(Configuration::End1) &&          valid_configuration(Configuration::End2)
			&&          valid_configuration(Configuration::End3) &&          valid_configuration(Configuration::End4) && valid_configuration(Configuration::CornerPrime1)
			&& valid_configuration(Configuration::CornerPrime2) && valid_configuration(Configuration::CornerPrime3) && valid_configuration(Configuration::CornerPrime4)
			&&        valid_configuration(Configuration::ILine1) &&        valid_configuration(Configuration::ILine2) &&  valid_configuration(Configuration::TBonePrime1)
			&&  valid_configuration(Configuration::TBonePrime2) &&  valid_configuration(Configuration::TBonePrime3) &&  valid_configuration(Configuration::TBonePrime4)
			&&   valid_configuration(Configuration::MiddlePrime);
	}

	bool TileSet::valid_4x4_2x2() const
	{
		return valid_4x4() && valid_configuration(Configuration::Corner1) && valid_configuration(Configuration::Corner2)
				            && valid_configuration(Configuration::Corner3) && valid_configuration(Configuration::Corner4);
	}

	TileSet::TileDesc TileSet::get_tile_desc(PaintedTile tile, Transformation& transformation) const
	{
		Tile t = get_assignment(get_configuration(tile), transformation);
		transformation &= t.transformation;
		return tiles[t.tex_index];
	}

	TileSet::Tile TileSet::get_assignment(Configuration config, Transformation& transformation) const
	{
		auto it = assignment.find(config);
		if (it != assignment.end())
			return it->second;
		if (!valid_6())
			throw Error(ErrorCode::IncompleteTileset);
		switch (config)
		{
			case Configuration::End2:
				transformation &= Transformation::Rotate90;
				return assignment.find(Configuration::End1)->second;
			case Configuration::End3:
				transformation &= Transformation::ReflectX;
				return assignment.find(Configuration::End1)->second;
			case Configuration::End4:
			{
				auto it = assignment.find(Configuration::End2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				else
				{
					transformation &= Transformation::Rotate270;
					return assignment.find(Configuration::End1)->second;
				}
			}
			case Configuration::Corner1:
				return assignment.find(Configuration::CornerPrime1)->second;
			case Configuration::Corner2:
			{
				auto it = assignment.find(Configuration::Corner1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::CornerPrime2);
				if (it != assignment.end())
					return it->second;
				else
				{
					transformation &= Transformation::ReflectX;
					return assignment.find(Configuration::CornerPrime1)->second;
				}
			}
			case Configuration::Corner3:
			{
				auto it = assignment.find(Configuration::Corner2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				it = assignment.find(Configuration::Corner1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate180;
					return it->second;
				}
				it = assignment.find(Configuration::CornerPrime3);
				if (it != assignment.end())
					return it->second;
				it = assignment.find(Configuration::CornerPrime2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				transformation &= Transformation::Rotate180;
				return assignment.find(Configuration::CornerPrime1)->second;
			}
			case Configuration::Corner4:
			{
				auto it = assignment.find(Configuration::Corner1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				it = assignment.find(Configuration::CornerPrime4);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ReflectY;
				return assignment.find(Configuration::CornerPrime1)->second;
			}
			case Configuration::ILine2:
				transformation &= Transformation::Rotate180;
				return assignment.find(Configuration::ILine1)->second;
			case Configuration::TBone1:
				return assignment.find(Configuration::TBonePrime1)->second;
			case Configuration::TBone2:
			{
				auto it = assignment.find(Configuration::TBone1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate90;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePrime2);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::Rotate90;
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBone3:
			{
				auto it = assignment.find(Configuration::TBone1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePrime3);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ReflectX;
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBone4:
			{
				auto it = assignment.find(Configuration::TBone1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate270;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePrime4);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::Rotate270;
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::Middle:
				return assignment.find(Configuration::MiddlePrime)->second;
			case Configuration::CornerPrime2:
				transformation &= Transformation::ReflectX;
				return assignment.find(Configuration::CornerPrime1)->second;
			case Configuration::CornerPrime3:
			{
				auto it = assignment.find(Configuration::CornerPrime2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				else
				{
					transformation &= Transformation::Rotate180;
					return assignment.find(Configuration::CornerPrime1)->second;
				}
			}
			case Configuration::CornerPrime4:
				transformation &= Transformation::ReflectY;
				return assignment.find(Configuration::CornerPrime1)->second;
			case Configuration::TBonePlus1:
			{
				auto it = assignment.find(Configuration::TBoneMinus1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBonePlus2:
			{
				auto it = assignment.find(Configuration::TBonePlus1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate90;
					return it->second;
				}
				it = assignment.find(Configuration::TBoneMinus2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePrime2);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::Rotate90;
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBonePlus3:
			{
				auto it = assignment.find(Configuration::TBonePlus1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::TBoneMinus3);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePrime3);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ReflectX;
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBonePlus4:
			{
				auto it = assignment.find(Configuration::TBonePlus1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate270;
					return it->second;
				}
				it = assignment.find(Configuration::TBoneMinus4);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePrime4);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::Rotate270;
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBoneMinus1:
			{
				auto it = assignment.find(Configuration::TBonePlus1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBoneMinus2:
			{
				auto it = assignment.find(Configuration::TBoneMinus1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate90;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePlus2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePrime2);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::Rotate90;
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBoneMinus3:
			{
				auto it = assignment.find(Configuration::TBoneMinus1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePlus3);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePrime3);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ReflectX;
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBoneMinus4:
			{
				auto it = assignment.find(Configuration::TBoneMinus1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate270;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePlus4);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::TBonePrime4);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::Rotate270;
				return assignment.find(Configuration::TBonePrime1)->second;
			}
			case Configuration::TBonePrime2:
				transformation &= Transformation::Rotate90;
				return assignment.find(Configuration::TBonePrime1)->second;
			case Configuration::TBonePrime3:
				transformation &= Transformation::ReflectX;
				return assignment.find(Configuration::TBonePrime1)->second;
			case Configuration::TBonePrime4:
			{
				auto it = assignment.find(Configuration::TBonePrime2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				else
				{
					transformation &= Transformation::Rotate270;
					return assignment.find(Configuration::TBonePrime1)->second;
				}
			}
			case Configuration::MiddleCorner1:
			{
				auto it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleCorner2:
			{
				auto it = assignment.find(Configuration::MiddleCorner1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleCorner3:
			{
				auto it = assignment.find(Configuration::MiddleCorner2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				it = assignment.find(Configuration::MiddleCorner1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate180;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleCorner4:
			{
				auto it = assignment.find(Configuration::MiddleCorner1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleTBone1:
			{
				auto it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleTBone2:
			{
				auto it = assignment.find(Configuration::MiddleTBone1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate90;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleTBone3:
			{
				auto it = assignment.find(Configuration::MiddleTBone1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleTBone4:
			{
				auto it = assignment.find(Configuration::MiddleTBone2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				it = assignment.find(Configuration::MiddleTBone1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate270;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleAcross1:
			{
				auto it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleAcross2:
			{
				auto it = assignment.find(Configuration::MiddleAcross1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleDiagonal1:
			{
				auto it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleDiagonal2:
			{
				auto it = assignment.find(Configuration::MiddleDiagonal1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectX;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleDiagonal3:
			{
				auto it = assignment.find(Configuration::MiddleCorner2);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				it = assignment.find(Configuration::MiddleDiagonal1);
				if (it != assignment.end())
				{
					transformation &= Transformation::Rotate180;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
			case Configuration::MiddleDiagonal4:
			{
				auto it = assignment.find(Configuration::MiddleCorner1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ReflectY;
					return it->second;
				}
				it = assignment.find(Configuration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MiddlePrime)->second;
			}
		}
		throw Error(ErrorCode::IncompleteTileset);
	}

	TileSet::Configuration TileSet::get_configuration(PaintedTile tile)
	{
		if (tile.orthogonal[0])
		{
			if (tile.orthogonal[1])
			{
				if (tile.orthogonal[2])
				{
					if (tile.orthogonal[3])
					{
						if (tile.diagonal[0])
						{
							if (tile.diagonal[1])
							{
								if (tile.diagonal[2])
								{
									if (tile.diagonal[3])
										return Configuration::MiddlePrime;
									else
										return Configuration::MiddleDiagonal4;
								}
								else if (tile.diagonal[3])
									return Configuration::MiddleDiagonal3;
								else
									return Configuration::MiddleTBone2;
							}
							else if (tile.diagonal[2])
							{
								if (tile.diagonal[3])
									return Configuration::MiddleDiagonal2;
								else
									return Configuration::MiddleAcross1;
							}
							else if (tile.diagonal[3])
								return Configuration::MiddleTBone1;
							else
								return Configuration::MiddleCorner1;
						}
						else if (tile.diagonal[1])
						{
							if (tile.diagonal[2])
							{
								if (tile.diagonal[3])
									return Configuration::MiddleDiagonal1;
								else
									return Configuration::MiddleTBone3;
							}
							else if (tile.diagonal[3])
								return Configuration::MiddleAcross2;
							else
								return Configuration::MiddleCorner2;
						}
						else if (tile.diagonal[2])
						{
							if (tile.diagonal[3])
								return Configuration::MiddleTBone4;
							else
								return Configuration::MiddleCorner3;
						}
						else if (tile.diagonal[3])
							return Configuration::MiddleCorner4;
						else
							return Configuration::Middle;
					}
					else
					{
						if (tile.diagonal[0])
						{
							if (tile.diagonal[1])
								return Configuration::TBonePrime2;
							else
								return Configuration::TBoneMinus2;
						}
						else if (tile.diagonal[1])
							return Configuration::TBonePlus2;
						else
							return Configuration::TBone2;
					}
				}
				else if (tile.orthogonal[3])
				{
					if (tile.diagonal[0])
					{
						if (tile.diagonal[3])
							return Configuration::TBonePrime1;
						else
							return Configuration::TBonePlus1;
					}
					else if (tile.diagonal[3])
						return Configuration::TBoneMinus1;
					else
						return Configuration::TBone1;
				}
				else if (tile.diagonal[0])
					return Configuration::CornerPrime1;
				else
					return Configuration::Corner1;
			}
			else if (tile.orthogonal[2])
			{
				if (tile.orthogonal[3])
				{
					if (tile.diagonal[2])
					{
						if (tile.diagonal[3])
							return Configuration::TBonePrime4;
						else
							return Configuration::TBoneMinus4;
					}
					else if (tile.diagonal[3])
						return Configuration::TBonePlus4;
					else
						return Configuration::TBone4;
				}
				else
					return Configuration::ILine1;
			}
			else if (tile.orthogonal[3])
			{
				if (tile.diagonal[3])
					return Configuration::CornerPrime4;
				else
					return Configuration::Corner4;
			}
			else
				return Configuration::End1;
		}
		else if (tile.orthogonal[1])
		{
			if (tile.orthogonal[2])
			{
				if (tile.orthogonal[3])
				{
					if (tile.diagonal[1])
					{
						if (tile.diagonal[2])
							return Configuration::TBonePrime3;
						else
							return Configuration::TBoneMinus3;
					}
					else if (tile.diagonal[2])
						return Configuration::TBonePlus3;
					else
						return Configuration::TBone3;
				}
				else
				{
					if (tile.diagonal[1])
						return Configuration::CornerPrime2;
					else
						return Configuration::Corner2;
				}
			}
			else if (tile.orthogonal[3])
				return Configuration::ILine2;
			else
				return Configuration::End2;
		}
		else if (tile.orthogonal[2])
		{
			if (tile.orthogonal[3])
			{
				if (tile.diagonal[2])
					return Configuration::CornerPrime3;
				else
					return Configuration::Corner3;
			}
			else
				return Configuration::End3;
		}
		else if (tile.orthogonal[3])
			return Configuration::End4;
		else
			return Configuration::Single;
	}
}
