#include "TileSetRegistry.h"

#include "core/context/Context.h"
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

		auto toml = load_toml(context::resource_file(file));
		auto node = toml["tileset"];
		auto toml_assignments = node["assignment"].as_array();
		
		std::vector<rendering::TileSet::Assignment> assignments;
		if (toml_assignments)
		{
			size_t _a_idx = 0;
			toml_assignments->for_each([&assignments, &_a_idx](auto&& node) {
				const size_t a_idx = _a_idx++;
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto _config = node["config"].value<std::string>();
					if (!_config)
					{
						LOG.warning(true, "REG") << LOG.source_info.full_source() << "Cannot parse tileset assignment #" << a_idx << " - missing \"config\" field." << LOG.nl;
						return;
					}
					auto _texture = node["texture"].value<std::string>();
					if (!_texture)
					{
						LOG.warning(true, "REG") << LOG.source_info.full_source() << "Cannot parse tileset assignment #" << a_idx << " - missing \"texture\" field." << LOG.nl;
						return;
					}

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
						size_t tr_idx = 0;
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
								else
									LOG.warning(true, "REG") << LOG.source_info.full_source() << "In tileset assignment #" << a_idx
															 << " transformation #" << tr_idx << ", unrecognized tile transformation \"" << tr << "\"." << LOG.nl;
							}
							else
								LOG.warning(true, "REG") << LOG.source_info.full_source() << "In tileset assignment #" << a_idx
														 << ", tile transformation #" << tr_idx << " is not a string." << LOG.nl;
							++tr_idx;
						}
					}

					const std::string& config = _config.value();

					// TODO v4 use static map
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
					{
						LOG.warning(true, "REG") << LOG.source_info.full_source() << "In tileset assignment #" << a_idx << ", unrecognized configuration \"" << config << "\"." << LOG.nl;
						return;
					}

					assignments.push_back(assignment);
				}
				else
					LOG.warning(true, "REG") << LOG.source_info.full_source() << "Cannot parse tileset assignment #" << a_idx << " - not a TOML table." << LOG.nl;
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
