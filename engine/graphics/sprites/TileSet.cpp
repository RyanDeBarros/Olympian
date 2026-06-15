#include "TileSet.h"

#include "core/util/Parser.h"
#include "core/util/Logger.h"

#include "definitions/Keys.h"
#include "util/Parser.h"

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
			this->assignments[a.config] = tile;
		}
	}

	TileSet::TileDesc TileSet::get_tile_desc(const detail::TileConfigGrid tile, detail::TileTransformation& transformation) const
	{
		std::unordered_set<detail::TileConfig> fallbacks_seen{};
		if (auto t = get_assignment(detail::tile_config_from_grid(tile), transformation, fallbacks_seen))
		{
			transformation |= t->transformation;
			return tiles[t->tex_index];
		}
		else
		{
			std::stringstream ss;
			ss << "[" << tile[0][0] << ", " << tile[0][1] << ", " << tile[0][2] << "]";
			ss << "[" << tile[1][0] << ", ---, " << tile[1][2] << "]";
			ss << "[" << tile[2][0] << ", " << tile[2][1] << ", " << tile[2][2] << "]";

			// TODO v9 logger api is too restrictive - allow for passing to LOG directly. Just revamp the whole system
			_OLY_ENGINE_LOG_ERROR("Tileset") << "Tileset cannot resolve tile configuration [" << ss.str() << "]" << LOG.endl;
			throw Error(ErrorCode::IncompleteTileset);
		}
	}

	std::optional<TileSet::Tile> TileSet::get_assignment(detail::TileConfig config, detail::TileTransformation& transformation, std::unordered_set<detail::TileConfig>& fallbacks_seen) const
	{
		auto it = assignments.find(config);
		if (it != assignments.end())
			return it->second;

		std::vector<std::pair<detail::TileConfig, detail::TileTransformation>> fallback_queue;
		int fallback_idx = 0;
		detail::TileConfig test_config = config;
		detail::TileTransformation test_transformation = transformation;
		while (detail::tile_config_fallback(test_config, test_transformation, fallback_idx))
		{
			auto it = assignments.find(test_config);
			if (it != assignments.end())
				return it->second;

			if (!fallbacks_seen.contains(test_config))
			{
				fallbacks_seen.insert(test_config);
				fallback_queue.push_back(std::make_pair(test_config, test_transformation));
				test_config = config;
				test_transformation = transformation;
			}

			++fallback_idx;
		}

		for (auto [c, t] : fallback_queue)
		{
			// TODO v8 transformations don't get transformed properly - rotations/reflections aren't combined because of bit flags
			if (auto tile = get_assignment(c, t, fallbacks_seen))
				return *tile;
		}
		return std::nullopt;
	}

	void TileSet::overload(TOMLNode node)
	{
		tiles.clear();
		this->assignments.clear();

		assets::Parser parser(node);
		auto toml_assignments = parser.optional<TOMLNode>(detail::Key::AssignmentArray)();
		if (toml_assignments && toml_assignments->as_table())
		{
			std::vector<rendering::TileSet::Assignment> assignments;

			size_t _a_idx = 0;
			for (auto&& [key, node] : *toml_assignments->as_table())
			{
				try
				{
					const size_t a_idx = _a_idx++;
					auto config = detail::stoi(key.str());
					if (!config)
						continue;

					assets::Parser parser((TOMLNode)node, { "in tileset assignment #", a_idx });

					const auto texture = parser.required<std::string>(detail::Key::TextureFile)();

					rendering::TileSet::Assignment assignment;
					assignment.config = static_cast<detail::TileConfig>(*config);

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
			}

			load_assignments(assignments);
		}
	}
}
