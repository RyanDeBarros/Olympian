#include "TileSetRegistry.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void TileSetRegistry::clear()
	{
		tilesets.clear();
	}

	rendering::TileSetRef TileSetRegistry::load_tileset(const std::string& file)
	{
		auto it = tilesets.find(file);
		if (it != tilesets.end())
			return it->second;

		auto toml = load_toml(context::context_filepath() + file);
		auto node = toml["tileset"];
		auto toml_assignments = node["assignment"].as_array();
		
		std::vector<rendering::TileSet::Assignment> assignments;
		if (toml_assignments)
		{
			toml_assignments->for_each([&assignments](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto _config = node["config"].value<std::string>();
					auto _texture = node["texture"].value<std::string>();
					if (!_config || !_texture)
						return;

					rendering::TileSet::Assignment assignment;

					assignment.desc.name = _texture.value();
					glm::vec4 uvs{};
					if (reg::parse_vec(node["uvs"].as_array(), uvs))
					{
						assignment.desc.uvs.x1 = uvs[0];
						assignment.desc.uvs.x2 = uvs[1];
						assignment.desc.uvs.y1 = uvs[2];
						assignment.desc.uvs.y2 = uvs[3];
					}

					if (auto transformations = node["trfm"].as_array())
					{
						for (const auto& trfm : *transformations)
						{
							if (auto transformation = trfm.value<std::string>())
							{
								const std::string tr = transformation.value();
								if (tr == "RX")
									assignment.transformation &= rendering::TileSet::Transformation::REFLECT_X;
								else if (tr == "RY")
									assignment.transformation &= rendering::TileSet::Transformation::REFLECT_Y;
								else if (tr == "R90")
									assignment.transformation &= rendering::TileSet::Transformation::ROTATE_90;
								else if (tr == "R180")
									assignment.transformation &= rendering::TileSet::Transformation::ROTATE_180;
								else if (tr == "R270")
									assignment.transformation &= rendering::TileSet::Transformation::ROTATE_270;
							}
						}
					}

					const std::string& config = _config.value();

					if (config == "S")
						assignment.config = rendering::TileSet::Configuration::SINGLE;
					else if (config == "E1")
						assignment.config = rendering::TileSet::Configuration::END_1;
					else if (config == "E2")
						assignment.config = rendering::TileSet::Configuration::END_2;
					else if (config == "E3")
						assignment.config = rendering::TileSet::Configuration::END_3;
					else if (config == "E4")
						assignment.config = rendering::TileSet::Configuration::END_4;
					else if (config == "C1")
						assignment.config = rendering::TileSet::Configuration::CORNER_1;
					else if (config == "C2")
						assignment.config = rendering::TileSet::Configuration::CORNER_2;
					else if (config == "C3")
						assignment.config = rendering::TileSet::Configuration::CORNER_3;
					else if (config == "C4")
						assignment.config = rendering::TileSet::Configuration::CORNER_4;
					else if (config == "I1")
						assignment.config = rendering::TileSet::Configuration::ILINE_1;
					else if (config == "I2")
						assignment.config = rendering::TileSet::Configuration::ILINE_2;
					else if (config == "T1")
						assignment.config = rendering::TileSet::Configuration::TBONE_1;
					else if (config == "T2")
						assignment.config = rendering::TileSet::Configuration::TBONE_2;
					else if (config == "T3")
						assignment.config = rendering::TileSet::Configuration::TBONE_3;
					else if (config == "T4")
						assignment.config = rendering::TileSet::Configuration::TBONE_4;
					else if (config == "M")
						assignment.config = rendering::TileSet::Configuration::MIDDLE;
					else if (config == "C1'")
						assignment.config = rendering::TileSet::Configuration::CORNER_PRIME_1;
					else if (config == "C2'")
						assignment.config = rendering::TileSet::Configuration::CORNER_PRIME_2;
					else if (config == "C3'")
						assignment.config = rendering::TileSet::Configuration::CORNER_PRIME_3;
					else if (config == "C4'")
						assignment.config = rendering::TileSet::Configuration::CORNER_PRIME_4;
					else if (config == "T1+")
						assignment.config = rendering::TileSet::Configuration::TBONE_PLUS_1;
					else if (config == "T2+")
						assignment.config = rendering::TileSet::Configuration::TBONE_PLUS_2;
					else if (config == "T3+")
						assignment.config = rendering::TileSet::Configuration::TBONE_PLUS_3;
					else if (config == "T4+")
						assignment.config = rendering::TileSet::Configuration::TBONE_PLUS_4;
					else if (config == "T1-")
						assignment.config = rendering::TileSet::Configuration::TBONE_MINUS_1;
					else if (config == "T2-")
						assignment.config = rendering::TileSet::Configuration::TBONE_MINUS_2;
					else if (config == "T3-")
						assignment.config = rendering::TileSet::Configuration::TBONE_MINUS_3;
					else if (config == "T4-")
						assignment.config = rendering::TileSet::Configuration::TBONE_MINUS_4;
					else if (config == "T1'")
						assignment.config = rendering::TileSet::Configuration::TBONE_PRIME_1;
					else if (config == "T2'")
						assignment.config = rendering::TileSet::Configuration::TBONE_PRIME_2;
					else if (config == "T3'")
						assignment.config = rendering::TileSet::Configuration::TBONE_PRIME_3;
					else if (config == "T4'")
						assignment.config = rendering::TileSet::Configuration::TBONE_PRIME_4;
					else if (config == "MC1")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_CORNER_1;
					else if (config == "MC2")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_CORNER_2;
					else if (config == "MC3")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_CORNER_3;
					else if (config == "MC4")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_CORNER_4;
					else if (config == "MT1")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_TBONE_1;
					else if (config == "MT2")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_TBONE_2;
					else if (config == "MT3")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_TBONE_3;
					else if (config == "MT4")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_TBONE_4;
					else if (config == "MA1")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_ACROSS_1;
					else if (config == "MA2")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_ACROSS_2;
					else if (config == "MD1")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_DIAGONAL_1;
					else if (config == "MD2")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_DIAGONAL_2;
					else if (config == "MD3")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_DIAGONAL_3;
					else if (config == "MD4")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_DIAGONAL_4;
					else if (config == "M'")
						assignment.config = rendering::TileSet::Configuration::MIDDLE_PRIME;
					else
						return;

					assignments.push_back(assignment);
				}
				});
		}

		rendering::TileSetRef tileset(assignments);
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			tilesets.emplace(file, tileset);
		return tileset;
	}

	void TileSetRegistry::free_tileset(const std::string& file)
	{
		tilesets.erase(file);
	}
}
