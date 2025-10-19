#include "TileSetRegistry.h"

#include "core/util/LoggerOperators.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void TileSetRegistry::clear()
	{
		tilesets.clear();
	}

	rendering::TileSetRef TileSetRegistry::load_tileset(const ResourcePath& file)
	{
		auto it = tilesets.find(file);
		if (it != tilesets.end())
			return it->second;

		auto toml = load_toml(file);
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

					int config = 0;
					if (parse_int(_config, config))
					{
						if (config >= 0 && config < (int64_t)rendering::TileSet::Configuration::_COUNT)
							assignment.config = (rendering::TileSet::Configuration)config;
						else
						{
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In tileset assignment #" << a_idx
														 << ", unrecognized configuration #" << config << "." << LOG.nl;
							return;
						}
					}
					else
					{
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse tileset assignment #" << a_idx
													 << " - \"config\" field is missing or not an int." << LOG.nl;
						return;
					}

					assignment.desc.file = _texture.value();
					glm::vec4 uvs{};
					if (reg::parse_vec(node["uvs"], uvs))
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

	void TileSetRegistry::free_tileset(const ResourcePath& file)
	{
		tilesets.erase(file);
	}
}
