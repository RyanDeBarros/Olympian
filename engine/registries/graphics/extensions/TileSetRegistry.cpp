#include "TileSetRegistry.h"

#include "core/context/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	// TODO v4 in assets, just use enum value instead of string.
	static constexpr std::array<std::pair<std::string_view, rendering::TileSet::Configuration>, (size_t)rendering::TileSet::Configuration::_COUNT> config_lut = { {
		{ "S", rendering::TileSet::Configuration::SINGLE },
		{ "E1", rendering::TileSet::Configuration::END_1 },
		{ "E2", rendering::TileSet::Configuration::END_2 },
		{ "E3", rendering::TileSet::Configuration::END_3 },
		{ "E4", rendering::TileSet::Configuration::END_4 },
		{ "C1", rendering::TileSet::Configuration::CORNER_1 },
		{ "C2", rendering::TileSet::Configuration::CORNER_2 },
		{ "C3", rendering::TileSet::Configuration::CORNER_3 },
		{ "C4", rendering::TileSet::Configuration::CORNER_4 },
		{ "I1", rendering::TileSet::Configuration::ILINE_1 },
		{ "I2", rendering::TileSet::Configuration::ILINE_2 },
		{ "T1", rendering::TileSet::Configuration::TBONE_1 },
		{ "T2", rendering::TileSet::Configuration::TBONE_2 },
		{ "T3", rendering::TileSet::Configuration::TBONE_3 },
		{ "T4", rendering::TileSet::Configuration::TBONE_4 },
		{ "M", rendering::TileSet::Configuration::MIDDLE },
		{ "C1'", rendering::TileSet::Configuration::CORNER_PRIME_1 },
		{ "C2'", rendering::TileSet::Configuration::CORNER_PRIME_2 },
		{ "C3'", rendering::TileSet::Configuration::CORNER_PRIME_3 },
		{ "C4'", rendering::TileSet::Configuration::CORNER_PRIME_4 },
		{ "T1+", rendering::TileSet::Configuration::TBONE_PLUS_1 },
		{ "T2+", rendering::TileSet::Configuration::TBONE_PLUS_2 },
		{ "T3+", rendering::TileSet::Configuration::TBONE_PLUS_3 },
		{ "T4+", rendering::TileSet::Configuration::TBONE_PLUS_4 },
		{ "T1-", rendering::TileSet::Configuration::TBONE_MINUS_1 },
		{ "T2-", rendering::TileSet::Configuration::TBONE_MINUS_2 },
		{ "T3-", rendering::TileSet::Configuration::TBONE_MINUS_3 },
		{ "T4-", rendering::TileSet::Configuration::TBONE_MINUS_4 },
		{ "T1'", rendering::TileSet::Configuration::TBONE_PRIME_1 },
		{ "T2'", rendering::TileSet::Configuration::TBONE_PRIME_2 },
		{ "T3'", rendering::TileSet::Configuration::TBONE_PRIME_3 },
		{ "T4'", rendering::TileSet::Configuration::TBONE_PRIME_4 },
		{ "MC1", rendering::TileSet::Configuration::MIDDLE_CORNER_1 },
		{ "MC2", rendering::TileSet::Configuration::MIDDLE_CORNER_2 },
		{ "MC3", rendering::TileSet::Configuration::MIDDLE_CORNER_3 },
		{ "MC4", rendering::TileSet::Configuration::MIDDLE_CORNER_4 },
		{ "MT1", rendering::TileSet::Configuration::MIDDLE_TBONE_1 },
		{ "MT2", rendering::TileSet::Configuration::MIDDLE_TBONE_2 },
		{ "MT3", rendering::TileSet::Configuration::MIDDLE_TBONE_3 },
		{ "MT4", rendering::TileSet::Configuration::MIDDLE_TBONE_4 },
		{ "MA1", rendering::TileSet::Configuration::MIDDLE_ACROSS_1 },
		{ "MA2", rendering::TileSet::Configuration::MIDDLE_ACROSS_2 },
		{ "MD1", rendering::TileSet::Configuration::MIDDLE_DIAGONAL_1 },
		{ "MD2", rendering::TileSet::Configuration::MIDDLE_DIAGONAL_2 },
		{ "MD3", rendering::TileSet::Configuration::MIDDLE_DIAGONAL_3 },
		{ "MD4", rendering::TileSet::Configuration::MIDDLE_DIAGONAL_4 },
		{ "M'", rendering::TileSet::Configuration::MIDDLE_PRIME },
	} };

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
		if (!node.as_table())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Cannot load tileset \"" << file << "\" - missing \"tileset\" table." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing tileset [" << (src ? *src : "") << "]." << LOG.nl;
		}

		auto toml_assignments = node["assignment"].as_array();
		
		std::vector<rendering::TileSet::Assignment> assignments;
		if (toml_assignments)
		{
			size_t _a_idx = 0;
			toml_assignments->for_each([&assignments, &_a_idx](auto&& node) {
				const size_t a_idx = _a_idx++;
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto _texture = node["texture"].value<std::string>();
					if (!_texture)
					{
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse tileset assignment #" << a_idx
													 << " - missing \"texture\" field." << LOG.nl;
						return;
					}

					auto _config = node["config"];
					if (!_config)
					{
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse tileset assignment #" << a_idx
													 << " - missing \"config\" field." << LOG.nl;
						return;
					}

					rendering::TileSet::Assignment assignment;

					if (auto config = _config.value<int64_t>())
					{
						if (*config >= 0 && *config < (int64_t)rendering::TileSet::Configuration::_COUNT)
							assignment.config = (rendering::TileSet::Configuration)*config;
						else
						{
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In tileset assignment #" << a_idx
														 << ", unrecognized configuration #" << *config << "." << LOG.nl;
							return;
						}
					}
					else if (auto config = _config.value<std::string>())
					{
						auto config_it = std::find_if(config_lut.begin(), config_lut.end(), [&config](const auto& pair) { return pair.first == *config; });
						if (config_it != config_lut.end())
							assignment.config = config_it->second;
						else
						{
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In tileset assignment #" << a_idx
														 << ", unrecognized configuration \"" << *config << "\"." << LOG.nl;
							return;
						}
					}
					else
					{
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse tileset assignment #" << a_idx
													 << " - \"config\" field is neither int nor string." << LOG.nl;
						return;
					}

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
									OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In tileset assignment #" << a_idx
																 << " transformation #" << tr_idx << ", unrecognized tile transformation \"" << tr << "\"." << LOG.nl;
							}
							else
								OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In tileset assignment #" << a_idx
															 << ", tile transformation #" << tr_idx << " is not a string." << LOG.nl;
							++tr_idx;
						}
					}

					assignments.push_back(assignment);
				}
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse tileset assignment #" << a_idx << " - not a TOML table." << LOG.nl;
				});
		}

		rendering::TileSetRef tileset(assignments);
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			tilesets.emplace(file, tileset);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Tileset [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return tileset;
	}

	void TileSetRegistry::free_tileset(const std::string& file)
	{
		tilesets.erase(file);
	}
}
