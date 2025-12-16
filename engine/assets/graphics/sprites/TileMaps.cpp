#include "TileMaps.h"

#include "core/context/rendering/Tilesets.h"
#include "assets/Loader.h"

namespace oly::assets
{
	rendering::TileMap load_tilemap(TOMLNode node, const char* source)
	{
		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "Parsing tilemap [" << (source ? source : "") << "]..." << LOG.nl;

		rendering::TileMap tilemap;
		if (auto transformer = node["transformer"])
		{
			tilemap.set_local() = Transform2D::load(transformer);
			tilemap.set_transformer().set_modifier() = assets::load_transform_modifier_2d(transformer["modifier"]);
		}

		if (auto toml_layers = node["layer"].as_array())
		{
			size_t _layer_idx = 0;
			toml_layers->for_each([&tilemap, &_layer_idx](auto&& _node) {
				const size_t layer_idx = _layer_idx++;
				TOMLNode node = (TOMLNode)_node;

				auto tileset = node["tileset"].value<std::string>();
				if (!tileset)
				{
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot parse tilemap layer #" << layer_idx << " - missing \"tileset\" string field." << LOG.nl;
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
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "In tilemap layer #" << layer_idx
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

		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "...Tilemap [" << (source ? source : "") << "] parsed." << LOG.nl;

		return tilemap;
	}
}
