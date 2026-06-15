#include "TileSet.h"

#include "core/util/Parser.h"
#include "core/util/Logger.h"

#include "definitions/Keys.h"

// TODO v9 after editor implementation for tileset/tilemap asset editor, define LUTs to simplify get_assignment()/get_configuration()

namespace oly::rendering
{
	TileSet::TileSet(const std::vector<Assignment>& assignments)
	{
		load_assignments(assignments);
	}

	void TileSet::load_assignments(const std::vector<Assignment>& assignments)
	{
		for (const Assignment& a : assignments)
		{
			auto texture_it = std::find(tiles.begin(), tiles.end(), a.desc);
			Tile tile{ .tex_index = size_t(texture_it - tiles.begin()) };
			if (texture_it == tiles.end())
				tiles.push_back(a.desc);
			tile.transformation |= a.transformation;
			assignment[a.config] = tile;
		}
	}

	bool TileSet::valid_configuration(detail::TileConfiguration configuration) const
	{
		auto it = assignment.find(configuration);
		if (it == assignment.end())
			return false;
		else
			return it->second.tex_index < tiles.size();
	}

	bool TileSet::valid_6() const
	{
		return valid_configuration(detail::TileConfiguration::Single) && valid_configuration(detail::TileConfiguration::End1)		 && valid_configuration(detail::TileConfiguration::CornerPrime1)
			&& valid_configuration(detail::TileConfiguration::ILine1) && valid_configuration(detail::TileConfiguration::TBonePrime1) && valid_configuration(detail::TileConfiguration::MiddlePrime);
	}

	bool TileSet::valid_4x4() const
	{
		return valid_configuration(detail::TileConfiguration::Single)		&& valid_configuration(detail::TileConfiguration::End1)			&& valid_configuration(detail::TileConfiguration::End2)
			&& valid_configuration(detail::TileConfiguration::End3)			&& valid_configuration(detail::TileConfiguration::End4)			&& valid_configuration(detail::TileConfiguration::CornerPrime1)
			&& valid_configuration(detail::TileConfiguration::CornerPrime2)	&& valid_configuration(detail::TileConfiguration::CornerPrime3)	&& valid_configuration(detail::TileConfiguration::CornerPrime4)
			&& valid_configuration(detail::TileConfiguration::ILine1)		&& valid_configuration(detail::TileConfiguration::ILine2)		&& valid_configuration(detail::TileConfiguration::TBonePrime1)
			&& valid_configuration(detail::TileConfiguration::TBonePrime2)	&& valid_configuration(detail::TileConfiguration::TBonePrime3)	&& valid_configuration(detail::TileConfiguration::TBonePrime4)
			&& valid_configuration(detail::TileConfiguration::MiddlePrime);
	}

	bool TileSet::valid_4x4_2x2() const
	{
		return valid_4x4() && valid_configuration(detail::TileConfiguration::Corner1) && valid_configuration(detail::TileConfiguration::Corner2)
							&& valid_configuration(detail::TileConfiguration::Corner3) && valid_configuration(detail::TileConfiguration::Corner4);
	}

	TileSet::TileDesc TileSet::get_tile_desc(PaintedTile tile, detail::TileTransformation& transformation) const
	{
		Tile t = get_assignment(get_configuration(tile), transformation);
		transformation |= t.transformation;
		return tiles[t.tex_index];
	}

	TileSet::Tile TileSet::get_assignment(detail::TileConfiguration config, detail::TileTransformation& transformation) const
	{
		auto it = assignment.find(config);
		if (it != assignment.end())
			return it->second;
		if (!valid_6())
			throw Error(ErrorCode::IncompleteTileset);
		switch (config)
		{
			case detail::TileConfiguration::End2:
				transformation |= detail::TileTransformation::Rotate90;
				return assignment.find(detail::TileConfiguration::End1)->second;
			case detail::TileConfiguration::End3:
				transformation |= detail::TileTransformation::ReflectX;
				return assignment.find(detail::TileConfiguration::End1)->second;
			case detail::TileConfiguration::End4:
			{
				auto it = assignment.find(detail::TileConfiguration::End2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				else
				{
					transformation |= detail::TileTransformation::Rotate270;
					return assignment.find(detail::TileConfiguration::End1)->second;
				}
			}
			case detail::TileConfiguration::Corner1:
				return assignment.find(detail::TileConfiguration::CornerPrime1)->second;
			case detail::TileConfiguration::Corner2:
			{
				auto it = assignment.find(detail::TileConfiguration::Corner1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::CornerPrime2);
				if (it != assignment.end())
					return it->second;
				else
				{
					transformation |= detail::TileTransformation::ReflectX;
					return assignment.find(detail::TileConfiguration::CornerPrime1)->second;
				}
			}
			case detail::TileConfiguration::Corner3:
			{
				auto it = assignment.find(detail::TileConfiguration::Corner2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Corner1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate180;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::CornerPrime3);
				if (it != assignment.end())
					return it->second;
				it = assignment.find(detail::TileConfiguration::CornerPrime2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				transformation |= detail::TileTransformation::Rotate180;
				return assignment.find(detail::TileConfiguration::CornerPrime1)->second;
			}
			case detail::TileConfiguration::Corner4:
			{
				auto it = assignment.find(detail::TileConfiguration::Corner1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::CornerPrime4);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::ReflectY;
				return assignment.find(detail::TileConfiguration::CornerPrime1)->second;
			}
			case detail::TileConfiguration::ILine2:
				transformation |= detail::TileTransformation::Rotate180;
				return assignment.find(detail::TileConfiguration::ILine1)->second;
			case detail::TileConfiguration::TBone1:
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			case detail::TileConfiguration::TBone2:
			{
				auto it = assignment.find(detail::TileConfiguration::TBone1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate90;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePrime2);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::Rotate90;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBone3:
			{
				auto it = assignment.find(detail::TileConfiguration::TBone1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePrime3);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::ReflectX;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBone4:
			{
				auto it = assignment.find(detail::TileConfiguration::TBone1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate270;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePrime4);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::Rotate270;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::Middle:
				return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			case detail::TileConfiguration::CornerPrime2:
				transformation |= detail::TileTransformation::ReflectX;
				return assignment.find(detail::TileConfiguration::CornerPrime1)->second;
			case detail::TileConfiguration::CornerPrime3:
			{
				auto it = assignment.find(detail::TileConfiguration::CornerPrime2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				else
				{
					transformation |= detail::TileTransformation::Rotate180;
					return assignment.find(detail::TileConfiguration::CornerPrime1)->second;
				}
			}
			case detail::TileConfiguration::CornerPrime4:
				transformation |= detail::TileTransformation::ReflectY;
				return assignment.find(detail::TileConfiguration::CornerPrime1)->second;
			case detail::TileConfiguration::TBonePlus1:
			{
				auto it = assignment.find(detail::TileConfiguration::TBoneMinus1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBonePlus2:
			{
				auto it = assignment.find(detail::TileConfiguration::TBonePlus1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate90;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBoneMinus2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePrime2);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::Rotate90;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBonePlus3:
			{
				auto it = assignment.find(detail::TileConfiguration::TBonePlus1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBoneMinus3);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePrime3);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::ReflectX;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBonePlus4:
			{
				auto it = assignment.find(detail::TileConfiguration::TBonePlus1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate270;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBoneMinus4);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePrime4);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::Rotate270;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBoneMinus1:
			{
				auto it = assignment.find(detail::TileConfiguration::TBonePlus1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBoneMinus2:
			{
				auto it = assignment.find(detail::TileConfiguration::TBoneMinus1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate90;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePlus2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePrime2);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::Rotate90;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBoneMinus3:
			{
				auto it = assignment.find(detail::TileConfiguration::TBoneMinus1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePlus3);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePrime3);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::ReflectX;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBoneMinus4:
			{
				auto it = assignment.find(detail::TileConfiguration::TBoneMinus1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate270;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePlus4);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::TBonePrime4);
				if (it != assignment.end())
					return it->second;
				transformation |= detail::TileTransformation::Rotate270;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			}
			case detail::TileConfiguration::TBonePrime2:
				transformation |= detail::TileTransformation::Rotate90;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			case detail::TileConfiguration::TBonePrime3:
				transformation |= detail::TileTransformation::ReflectX;
				return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
			case detail::TileConfiguration::TBonePrime4:
			{
				auto it = assignment.find(detail::TileConfiguration::TBonePrime2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				else
				{
					transformation |= detail::TileTransformation::Rotate270;
					return assignment.find(detail::TileConfiguration::TBonePrime1)->second;
				}
			}
			case detail::TileConfiguration::MiddleCorner1:
			{
				auto it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleCorner2:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleCorner1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleCorner3:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleCorner2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::MiddleCorner1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate180;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleCorner4:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleCorner1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleTBone1:
			{
				auto it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleTBone2:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleTBone1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate90;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleTBone3:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleTBone1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleTBone4:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleTBone2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::MiddleTBone1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate270;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleAcross1:
			{
				auto it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleAcross2:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleAcross1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleDiagonal1:
			{
				auto it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleDiagonal2:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleDiagonal1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectX;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleDiagonal3:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleCorner2);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::MiddleDiagonal1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::Rotate180;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
			case detail::TileConfiguration::MiddleDiagonal4:
			{
				auto it = assignment.find(detail::TileConfiguration::MiddleCorner1);
				if (it != assignment.end())
				{
					transformation |= detail::TileTransformation::ReflectY;
					return it->second;
				}
				it = assignment.find(detail::TileConfiguration::Middle);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(detail::TileConfiguration::MiddlePrime)->second;
			}
		}
		throw Error(ErrorCode::IncompleteTileset);
	}

	detail::TileConfiguration TileSet::get_configuration(PaintedTile tile)
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
										return detail::TileConfiguration::MiddlePrime;
									else
										return detail::TileConfiguration::MiddleDiagonal4;
								}
								else if (tile.diagonal[3])
									return detail::TileConfiguration::MiddleDiagonal3;
								else
									return detail::TileConfiguration::MiddleTBone2;
							}
							else if (tile.diagonal[2])
							{
								if (tile.diagonal[3])
									return detail::TileConfiguration::MiddleDiagonal2;
								else
									return detail::TileConfiguration::MiddleAcross1;
							}
							else if (tile.diagonal[3])
								return detail::TileConfiguration::MiddleTBone1;
							else
								return detail::TileConfiguration::MiddleCorner1;
						}
						else if (tile.diagonal[1])
						{
							if (tile.diagonal[2])
							{
								if (tile.diagonal[3])
									return detail::TileConfiguration::MiddleDiagonal1;
								else
									return detail::TileConfiguration::MiddleTBone3;
							}
							else if (tile.diagonal[3])
								return detail::TileConfiguration::MiddleAcross2;
							else
								return detail::TileConfiguration::MiddleCorner2;
						}
						else if (tile.diagonal[2])
						{
							if (tile.diagonal[3])
								return detail::TileConfiguration::MiddleTBone4;
							else
								return detail::TileConfiguration::MiddleCorner3;
						}
						else if (tile.diagonal[3])
							return detail::TileConfiguration::MiddleCorner4;
						else
							return detail::TileConfiguration::Middle;
					}
					else
					{
						if (tile.diagonal[0])
						{
							if (tile.diagonal[1])
								return detail::TileConfiguration::TBonePrime2;
							else
								return detail::TileConfiguration::TBoneMinus2;
						}
						else if (tile.diagonal[1])
							return detail::TileConfiguration::TBonePlus2;
						else
							return detail::TileConfiguration::TBone2;
					}
				}
				else if (tile.orthogonal[3])
				{
					if (tile.diagonal[0])
					{
						if (tile.diagonal[3])
							return detail::TileConfiguration::TBonePrime1;
						else
							return detail::TileConfiguration::TBonePlus1;
					}
					else if (tile.diagonal[3])
						return detail::TileConfiguration::TBoneMinus1;
					else
						return detail::TileConfiguration::TBone1;
				}
				else if (tile.diagonal[0])
					return detail::TileConfiguration::CornerPrime1;
				else
					return detail::TileConfiguration::Corner1;
			}
			else if (tile.orthogonal[2])
			{
				if (tile.orthogonal[3])
				{
					if (tile.diagonal[2])
					{
						if (tile.diagonal[3])
							return detail::TileConfiguration::TBonePrime4;
						else
							return detail::TileConfiguration::TBoneMinus4;
					}
					else if (tile.diagonal[3])
						return detail::TileConfiguration::TBonePlus4;
					else
						return detail::TileConfiguration::TBone4;
				}
				else
					return detail::TileConfiguration::ILine1;
			}
			else if (tile.orthogonal[3])
			{
				if (tile.diagonal[3])
					return detail::TileConfiguration::CornerPrime4;
				else
					return detail::TileConfiguration::Corner4;
			}
			else
				return detail::TileConfiguration::End1;
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
							return detail::TileConfiguration::TBonePrime3;
						else
							return detail::TileConfiguration::TBoneMinus3;
					}
					else if (tile.diagonal[2])
						return detail::TileConfiguration::TBonePlus3;
					else
						return detail::TileConfiguration::TBone3;
				}
				else
				{
					if (tile.diagonal[1])
						return detail::TileConfiguration::CornerPrime2;
					else
						return detail::TileConfiguration::Corner2;
				}
			}
			else if (tile.orthogonal[3])
				return detail::TileConfiguration::ILine2;
			else
				return detail::TileConfiguration::End2;
		}
		else if (tile.orthogonal[2])
		{
			if (tile.orthogonal[3])
			{
				if (tile.diagonal[2])
					return detail::TileConfiguration::CornerPrime3;
				else
					return detail::TileConfiguration::Corner3;
			}
			else
				return detail::TileConfiguration::End3;
		}
		else if (tile.orthogonal[3])
			return detail::TileConfiguration::End4;
		else
			return detail::TileConfiguration::Single;
	}

	void TileSet::overload(TOMLNode node)
	{
		assets::Parser parser(node);
		auto toml_assignments = parser.optional<TOMLArray>(detail::Key::AssignmentArray)();
		if (!toml_assignments)
			return;

		std::vector<rendering::TileSet::Assignment> assignments;
		tiles.clear();
		assignment.clear();

		size_t _a_idx = 0;
		toml_assignments->for_each([&assignments, &_a_idx](auto&& node) {
			try
			{
				const size_t a_idx = _a_idx++;
				assets::Parser parser((TOMLNode)node, { "in tileset assignment #", a_idx });

				const auto texture = parser.required<std::string>(detail::Key::TextureFile)();
				const auto config = parser.required<detail::TileConfiguration>(detail::Key::Configuration)();

				rendering::TileSet::Assignment assignment;

				if (static_cast<int>(config) >= 0 && static_cast<int>(config) < static_cast<int>(detail::TileConfiguration::_c))
					assignment.config = config;
				else
				{
					_OLY_ENGINE_LOG_ERROR("CONTEXT") << "unrecognized configuration (" << static_cast<int>(config) << ") in tileset assignment #" << a_idx << LOG.nl;
					throw Error(ErrorCode::LoadAsset);
				}

				assignment.desc.file = detail::ResourcePath(texture);
				parser.optional(detail::Key::TextureIndex)(assignment.desc.file_index);
				if (auto uvs = parser.optional<glm::vec4>(detail::Key::UVvec4)())
				{
					assignment.desc.uvs.x1 = (*uvs)[0];
					assignment.desc.uvs.x2 = (*uvs)[1];
					assignment.desc.uvs.y1 = (*uvs)[2];
					assignment.desc.uvs.y2 = (*uvs)[3];
				}

				if (auto transformations = parser.optional<TOMLArray>(detail::Key::TransformationArray)())
				{
					size_t tr_idx = 0;
					for (auto& trfm : *transformations)
					{
						assets::Parser tr_parser((TOMLNode)trfm, { "in transformation #", tr_idx, " from tileset assignment #", a_idx });

						if (auto transformation = tr_parser.optional<detail::TileTransformation>(assets::NO_KEY)())
							assignment.transformation |= *transformation;

						++tr_idx;
					}
				}

				assignments.push_back(assignment);
			}
			catch (const Error& e)
			{
				if (e.code != ErrorCode::LoadAsset)
					throw;
			}
		});

		load_assignments(assignments);
	}
}
