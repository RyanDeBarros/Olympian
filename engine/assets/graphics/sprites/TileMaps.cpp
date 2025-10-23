#include "TileMaps.h"

#include "core/context/rendering/Tilesets.h"
#include "assets/Loader.h"

namespace oly::assets
{
	rendering::TileMap load_tilemap(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "Parsing tilemap [" << (src ? *src : "") << "]." << LOG.nl;
		}

		rendering::TileMap tilemap;
		if (auto transformer = node["transformer"])
		{
			tilemap.set_local() = assets::load_transform_2d(transformer);
			tilemap.set_transformer().set_modifier() = assets::load_transform_modifier_2d(transformer["modifier"]);
		}

		auto toml_layers = node["layer"].as_array();
		if (toml_layers)
		{
			size_t _layer_idx = 0;
			toml_layers->for_each([&tilemap, &_layer_idx](auto&& _node) {
				const size_t layer_idx = _layer_idx++;
				TOMLNode node = (TOMLNode)_node;

				auto tileset = node["tileset"].value<std::string>();
				if (!tileset)
				{
					OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot parse tilemap layer #" << layer_idx << " - missing \"tileset\" string field." << LOG.nl;
					return;
				}

				rendering::TileMapLayer layer;
				layer.tileset = context::load_tileset(*tileset);

				auto tiles = node["tiles"].as_array();
				if (tiles)
				{
					size_t tile_idx = 0;
					for (auto& toml_tile : *tiles)
					{
						glm::ivec2 tile{};
						if (parse_ivec((TOMLNode)toml_tile, tile))
							layer.paint_tile(tile);
						else
							OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "In tilemap layer #" << layer_idx
															<< ", cannot convert tile #" << tile_idx << " to vec2." << LOG.nl;
						++tile_idx;
					}
				}

				int z = 0;
				if (parse_int(node["z"], z))
					tilemap.register_layer(z, std::move(layer));
				else
					tilemap.register_layer(std::move(layer));
				});
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "Tilemap [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return tilemap;
	}
}
