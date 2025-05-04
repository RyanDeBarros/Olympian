#include "TileSet.h"

#include "core/base/Context.h"
#include "core/util/IO.h"
#include "registries/Loader.h"

namespace oly::rendering
{
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
		return  valid_configuration(Configuration::SINGLE) &&   valid_configuration(Configuration::END_1) && valid_configuration(Configuration::CORNER_PRIME_1)
			&& valid_configuration(Configuration::ILINE_1) && valid_configuration(Configuration::TBONE_PRIME_1) && valid_configuration(Configuration::MIDDLE_PRIME);
	}

	bool TileSet::valid_4x4() const
	{
		return         valid_configuration(Configuration::SINGLE) &&          valid_configuration(Configuration::END_1) &&          valid_configuration(Configuration::END_2)
			&&          valid_configuration(Configuration::END_3) &&          valid_configuration(Configuration::END_4) && valid_configuration(Configuration::CORNER_PRIME_1)
			&& valid_configuration(Configuration::CORNER_PRIME_2) && valid_configuration(Configuration::CORNER_PRIME_3) && valid_configuration(Configuration::CORNER_PRIME_4)
			&&        valid_configuration(Configuration::ILINE_1) &&        valid_configuration(Configuration::ILINE_2) &&  valid_configuration(Configuration::TBONE_PRIME_1)
			&&  valid_configuration(Configuration::TBONE_PRIME_2) &&  valid_configuration(Configuration::TBONE_PRIME_3) &&  valid_configuration(Configuration::TBONE_PRIME_4)
			&&   valid_configuration(Configuration::MIDDLE_PRIME);
	}

	bool TileSet::valid_4x4_2x2() const
	{
		return valid_4x4() && valid_configuration(Configuration::CORNER_1) && valid_configuration(Configuration::CORNER_2)
				            && valid_configuration(Configuration::CORNER_3) && valid_configuration(Configuration::CORNER_4);
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
			throw Error(ErrorCode::INCOMPLETE_TILESET);
		switch (config)
		{
			case Configuration::END_2:
				transformation &= Transformation::ROTATE_90;
				return assignment.find(Configuration::END_1)->second;
			case Configuration::END_3:
				transformation &= Transformation::REFLECT_X;
				return assignment.find(Configuration::END_1)->second;
			case Configuration::END_4:
			{
				auto it = assignment.find(Configuration::END_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				else
				{
					transformation &= Transformation::ROTATE_270;
					return assignment.find(Configuration::END_1)->second;
				}
			}
			case Configuration::CORNER_1:
				return assignment.find(Configuration::CORNER_PRIME_1)->second;
			case Configuration::CORNER_2:
			{
				auto it = assignment.find(Configuration::CORNER_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::CORNER_PRIME_2);
				if (it != assignment.end())
					return it->second;
				else
				{
					transformation &= Transformation::REFLECT_X;
					return assignment.find(Configuration::CORNER_PRIME_1)->second;
				}
			}
			case Configuration::CORNER_3:
			{
				auto it = assignment.find(Configuration::CORNER_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				it = assignment.find(Configuration::CORNER_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_180;
					return it->second;
				}
				it = assignment.find(Configuration::CORNER_PRIME_3);
				if (it != assignment.end())
					return it->second;
				it = assignment.find(Configuration::CORNER_PRIME_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				transformation &= Transformation::ROTATE_180;
				return assignment.find(Configuration::CORNER_PRIME_1)->second;
			}
			case Configuration::CORNER_4:
			{
				auto it = assignment.find(Configuration::CORNER_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				it = assignment.find(Configuration::CORNER_PRIME_4);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::REFLECT_Y;
				return assignment.find(Configuration::CORNER_PRIME_1)->second;
			}
			case Configuration::ILINE_2:
				transformation &= Transformation::ROTATE_180;
				return assignment.find(Configuration::ILINE_1)->second;
			case Configuration::TBONE_1:
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			case Configuration::TBONE_2:
			{
				auto it = assignment.find(Configuration::TBONE_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_90;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PRIME_2);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ROTATE_90;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_3:
			{
				auto it = assignment.find(Configuration::TBONE_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PRIME_3);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::REFLECT_X;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_4:
			{
				auto it = assignment.find(Configuration::TBONE_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_270;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PRIME_4);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ROTATE_270;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::MIDDLE:
				return assignment.find(Configuration::MIDDLE_PRIME)->second;
			case Configuration::CORNER_PRIME_2:
				transformation &= Transformation::REFLECT_X;
				return assignment.find(Configuration::CORNER_PRIME_1)->second;
			case Configuration::CORNER_PRIME_3:
			{
				auto it = assignment.find(Configuration::CORNER_PRIME_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				else
				{
					transformation &= Transformation::ROTATE_180;
					return assignment.find(Configuration::CORNER_PRIME_1)->second;
				}
			}
			case Configuration::CORNER_PRIME_4:
				transformation &= Transformation::REFLECT_Y;
				return assignment.find(Configuration::CORNER_PRIME_1)->second;
			case Configuration::TBONE_PLUS_1:
			{
				auto it = assignment.find(Configuration::TBONE_MINUS_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_PLUS_2:
			{
				auto it = assignment.find(Configuration::TBONE_PLUS_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_90;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_MINUS_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PRIME_2);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ROTATE_90;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_PLUS_3:
			{
				auto it = assignment.find(Configuration::TBONE_PLUS_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_MINUS_3);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PRIME_3);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::REFLECT_X;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_PLUS_4:
			{
				auto it = assignment.find(Configuration::TBONE_PLUS_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_270;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_MINUS_4);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PRIME_4);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ROTATE_270;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_MINUS_1:
			{
				auto it = assignment.find(Configuration::TBONE_PLUS_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_MINUS_2:
			{
				auto it = assignment.find(Configuration::TBONE_MINUS_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_90;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PLUS_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PRIME_2);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ROTATE_90;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_MINUS_3:
			{
				auto it = assignment.find(Configuration::TBONE_MINUS_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PLUS_3);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PRIME_3);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::REFLECT_X;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_MINUS_4:
			{
				auto it = assignment.find(Configuration::TBONE_MINUS_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_270;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PLUS_4);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::TBONE_PRIME_4);
				if (it != assignment.end())
					return it->second;
				transformation &= Transformation::ROTATE_270;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			}
			case Configuration::TBONE_PRIME_2:
				transformation &= Transformation::ROTATE_90;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			case Configuration::TBONE_PRIME_3:
				transformation &= Transformation::REFLECT_X;
				return assignment.find(Configuration::TBONE_PRIME_1)->second;
			case Configuration::TBONE_PRIME_4:
			{
				auto it = assignment.find(Configuration::TBONE_PRIME_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				else
				{
					transformation &= Transformation::ROTATE_270;
					return assignment.find(Configuration::TBONE_PRIME_1)->second;
				}
			}
			case Configuration::MIDDLE_CORNER_1:
			{
				auto it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_CORNER_2:
			{
				auto it = assignment.find(Configuration::MIDDLE_CORNER_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_CORNER_3:
			{
				auto it = assignment.find(Configuration::MIDDLE_CORNER_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE_CORNER_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_180;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_CORNER_4:
			{
				auto it = assignment.find(Configuration::MIDDLE_CORNER_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_TBONE_1:
			{
				auto it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_TBONE_2:
			{
				auto it = assignment.find(Configuration::MIDDLE_TBONE_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_90;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_TBONE_3:
			{
				auto it = assignment.find(Configuration::MIDDLE_TBONE_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_TBONE_4:
			{
				auto it = assignment.find(Configuration::MIDDLE_TBONE_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE_TBONE_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_270;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_ACROSS_1:
			{
				auto it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_ACROSS_2:
			{
				auto it = assignment.find(Configuration::MIDDLE_ACROSS_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_DIAGONAL_1:
			{
				auto it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_DIAGONAL_2:
			{
				auto it = assignment.find(Configuration::MIDDLE_DIAGONAL_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_X;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_DIAGONAL_3:
			{
				auto it = assignment.find(Configuration::MIDDLE_CORNER_2);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE_DIAGONAL_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::ROTATE_180;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
			case Configuration::MIDDLE_DIAGONAL_4:
			{
				auto it = assignment.find(Configuration::MIDDLE_CORNER_1);
				if (it != assignment.end())
				{
					transformation &= Transformation::REFLECT_Y;
					return it->second;
				}
				it = assignment.find(Configuration::MIDDLE);
				if (it != assignment.end())
					return it->second;
				else
					return assignment.find(Configuration::MIDDLE_PRIME)->second;
			}
		}
		throw Error(ErrorCode::INCOMPLETE_TILESET);
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
										return Configuration::MIDDLE_PRIME;
									else
										return Configuration::MIDDLE_DIAGONAL_4;
								}
								else if (tile.diagonal[3])
									return Configuration::MIDDLE_DIAGONAL_3;
								else
									return Configuration::MIDDLE_TBONE_2;
							}
							else if (tile.diagonal[2])
							{
								if (tile.diagonal[3])
									return Configuration::MIDDLE_DIAGONAL_2;
								else
									return Configuration::MIDDLE_ACROSS_1;
							}
							else if (tile.diagonal[3])
								return Configuration::MIDDLE_TBONE_1;
							else
								return Configuration::MIDDLE_CORNER_1;
						}
						else if (tile.diagonal[1])
						{
							if (tile.diagonal[2])
							{
								if (tile.diagonal[3])
									return Configuration::MIDDLE_DIAGONAL_1;
								else
									return Configuration::MIDDLE_TBONE_3;
							}
							else if (tile.diagonal[3])
								return Configuration::MIDDLE_ACROSS_2;
							else
								return Configuration::MIDDLE_CORNER_2;
						}
						else if (tile.diagonal[2])
						{
							if (tile.diagonal[3])
								return Configuration::MIDDLE_TBONE_4;
							else
								return Configuration::MIDDLE_CORNER_3;
						}
						else if (tile.diagonal[3])
							return Configuration::MIDDLE_CORNER_4;
						else
							return Configuration::MIDDLE;
					}
					else
					{
						if (tile.diagonal[0])
						{
							if (tile.diagonal[1])
								return Configuration::TBONE_PRIME_2;
							else
								return Configuration::TBONE_MINUS_2;
						}
						else if (tile.diagonal[1])
							return Configuration::TBONE_PLUS_2;
						else
							return Configuration::TBONE_2;
					}
				}
				else if (tile.orthogonal[3])
				{
					if (tile.diagonal[0])
					{
						if (tile.diagonal[3])
							return Configuration::TBONE_PRIME_1;
						else
							return Configuration::TBONE_PLUS_1;
					}
					else if (tile.diagonal[3])
						return Configuration::TBONE_MINUS_1;
					else
						return Configuration::TBONE_1;
				}
				else if (tile.diagonal[0])
					return Configuration::CORNER_PRIME_1;
				else
					return Configuration::CORNER_1;
			}
			else if (tile.orthogonal[2])
			{
				if (tile.orthogonal[3])
				{
					if (tile.diagonal[2])
					{
						if (tile.diagonal[3])
							return Configuration::TBONE_PRIME_4;
						else
							return Configuration::TBONE_MINUS_4;
					}
					else if (tile.diagonal[3])
						return Configuration::TBONE_PLUS_4;
					else
						return Configuration::TBONE_4;
				}
				else
					return Configuration::ILINE_1;
			}
			else if (tile.orthogonal[3])
			{
				if (tile.diagonal[3])
					return Configuration::CORNER_PRIME_4;
				else
					return Configuration::CORNER_4;
			}
			else
				return Configuration::END_1;
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
							return Configuration::TBONE_PRIME_3;
						else
							return Configuration::TBONE_MINUS_3;
					}
					else if (tile.diagonal[2])
						return Configuration::TBONE_PLUS_3;
					else
						return Configuration::TBONE_3;
				}
				else
				{
					if (tile.diagonal[1])
						return Configuration::CORNER_PRIME_2;
					else
						return Configuration::CORNER_2;
				}
			}
			else if (tile.orthogonal[3])
				return Configuration::ILINE_2;
			else
				return Configuration::END_2;
		}
		else if (tile.orthogonal[2])
		{
			if (tile.orthogonal[3])
			{
				if (tile.diagonal[2])
					return Configuration::CORNER_PRIME_3;
				else
					return Configuration::CORNER_3;
			}
			else
				return Configuration::END_3;
		}
		else if (tile.orthogonal[3])
			return Configuration::END_4;
		else
			return Configuration::SINGLE;
	}

	void TileSet::load(const toml::array& toml_assignments)
	{
		toml_assignments.for_each([this](auto&& node) {
			if constexpr (toml::is_table<decltype(node)>)
			{
				auto _config = node["config"].value<std::string>();
				auto _tex = node["tex"].value<std::string>();
				if (!_config || !_tex)
					return;

				TileDesc desc;
				desc.name = _tex.value();
				glm::vec4 uvs{};
				if (reg::parse_vec4(node, "uvs", uvs))
				{
					desc.uvs.x1 = uvs[0];
					desc.uvs.x2 = uvs[1];
					desc.uvs.y1 = uvs[2];
					desc.uvs.y2 = uvs[3];
				}
				Tile tile;
				auto texture_it = std::find(tiles.begin(), tiles.end(), desc);
				tile.tex_index = texture_it - tiles.begin();
				if (texture_it == tiles.end())
					tiles.push_back(desc);

				Configuration c;
				const std::string& config = _config.value();

				if (config == "S")
					c = Configuration::SINGLE;
				else if (config == "E1")
					c = Configuration::END_1;
				else if (config == "E2")
					c = Configuration::END_2;
				else if (config == "E3")
					c = Configuration::END_3;
				else if (config == "E4")
					c = Configuration::END_4;
				else if (config == "C1")
					c = Configuration::CORNER_1;
				else if (config == "C2")
					c = Configuration::CORNER_2;
				else if (config == "C3")
					c = Configuration::CORNER_3;
				else if (config == "C4")
					c = Configuration::CORNER_4;
				else if (config == "I1")
					c = Configuration::ILINE_1;
				else if (config == "I2")
					c = Configuration::ILINE_2;
				else if (config == "T1")
					c = Configuration::TBONE_1;
				else if (config == "T2")
					c = Configuration::TBONE_2;
				else if (config == "T3")
					c = Configuration::TBONE_3;
				else if (config == "T4")
					c = Configuration::TBONE_4;
				else if (config == "M")
					c = Configuration::MIDDLE;
				else if (config == "C1'")
					c = Configuration::CORNER_PRIME_1;
				else if (config == "C2'")
					c = Configuration::CORNER_PRIME_2;
				else if (config == "C3'")
					c = Configuration::CORNER_PRIME_3;
				else if (config == "C4'")
					c = Configuration::CORNER_PRIME_4;
				else if (config == "T1+")
					c = Configuration::TBONE_PLUS_1;
				else if (config == "T2+")
					c = Configuration::TBONE_PLUS_2;
				else if (config == "T3+")
					c = Configuration::TBONE_PLUS_3;
				else if (config == "T4+")
					c = Configuration::TBONE_PLUS_4;
				else if (config == "T1-")
					c = Configuration::TBONE_MINUS_1;
				else if (config == "T2-")
					c = Configuration::TBONE_MINUS_2;
				else if (config == "T3-")
					c = Configuration::TBONE_MINUS_3;
				else if (config == "T4-")
					c = Configuration::TBONE_MINUS_4;
				else if (config == "T1'")
					c = Configuration::TBONE_PRIME_1;
				else if (config == "T2'")
					c = Configuration::TBONE_PRIME_2;
				else if (config == "T3'")
					c = Configuration::TBONE_PRIME_3;
				else if (config == "T4'")
					c = Configuration::TBONE_PRIME_4;
				else if (config == "MC1")
					c = Configuration::MIDDLE_CORNER_1;
				else if (config == "MC2")
					c = Configuration::MIDDLE_CORNER_2;
				else if (config == "MC3")
					c = Configuration::MIDDLE_CORNER_3;
				else if (config == "MC4")
					c = Configuration::MIDDLE_CORNER_4;
				else if (config == "MT1")
					c = Configuration::MIDDLE_TBONE_1;
				else if (config == "MT2")
					c = Configuration::MIDDLE_TBONE_2;
				else if (config == "MT3")
					c = Configuration::MIDDLE_TBONE_3;
				else if (config == "MT4")
					c = Configuration::MIDDLE_TBONE_4;
				else if (config == "MA1")
					c = Configuration::MIDDLE_ACROSS_1;
				else if (config == "MA2")
					c = Configuration::MIDDLE_ACROSS_2;
				else if (config == "MD1")
					c = Configuration::MIDDLE_DIAGONAL_1;
				else if (config == "MD2")
					c = Configuration::MIDDLE_DIAGONAL_2;
				else if (config == "MD3")
					c = Configuration::MIDDLE_DIAGONAL_3;
				else if (config == "MD4")
					c = Configuration::MIDDLE_DIAGONAL_4;
				else if (config == "M'")
					c = Configuration::MIDDLE_PRIME;
				else
					return;

				if (auto transformations = node["trfm"].as_array())
				{
					for (const auto& trfm : *transformations)
					{
						if (auto transformation = trfm.value<std::string>())
						{
							const std::string tr = transformation.value();
							if (tr == "RX")
								tile.transformation &= Transformation::REFLECT_X;
							else if (tr == "RY")
								tile.transformation &= Transformation::REFLECT_Y;
							else if (tr == "R90")
								tile.transformation &= Transformation::ROTATE_90;
							else if (tr == "R180")
								tile.transformation &= Transformation::ROTATE_180;
							else if (tr == "R270")
								tile.transformation &= Transformation::ROTATE_270;
						}
					}
				}

				assignment[c] = tile;
			}
			});
	}
}
