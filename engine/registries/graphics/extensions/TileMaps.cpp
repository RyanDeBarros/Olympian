#include "TileMaps.h"

#include "core/context/rendering/Tilesets.h"
#include "registries/Loader.h"

namespace oly::reg
{
	rendering::TileMap load_tilemap(const TOMLNode& node)
	{
		params::TileMap params;

		params.local = reg::load_transform_2d(node, "transform");

		auto toml_layers = node["layer"].as_array();
		if (toml_layers)
		{
			size_t _layer_idx = 0;
			toml_layers->for_each([&params, &_layer_idx](auto&& node) {
				const size_t layer_idx = _layer_idx++;
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto tileset = node["tileset"].value<std::string>();
					if (!tileset)
					{
						LOG.warning(true, "REG") << LOG.source_info.full_source() << "Cannot parse tilemap layer #" << layer_idx << " - missing \"tileset\" string field." << LOG.nl;
						return;
					}

					params::TileMap::Layer lparams;
					lparams.tileset = tileset.value();

					auto tiles = node["tiles"].as_array();
					if (tiles)
					{
						size_t tile_idx = 0;
						for (const auto& toml_tile : *tiles)
						{
							if (auto _tile = toml_tile.as_array())
							{
								glm::ivec2 tile{};
								if (parse_ivec(_tile, tile))
									lparams.tiles.push_back(tile);
								else
									LOG.warning(true, "REG") << LOG.source_info.full_source() << "In tilemap layer #" << layer_idx
															  << ", cannot convert tile #" << tile_idx << " to vec2." << LOG.nl;
							}
							++tile_idx;
						}
					}

					if (auto z = node["z"].value<int64_t>())
						lparams.z = (int)z.value();

					params.layers.push_back(std::move(lparams));
				}
				else
					LOG.warning(true, "REG") << LOG.source_info.full_source() << "Cannot parse tilemap layer #" << layer_idx << " - not a TOML table." << LOG.nl;
				});
		}

		return load_tilemap(params);
	}

	rendering::TileMap load_tilemap(const params::TileMap& params)
	{
		rendering::TileMap tilemap;

		tilemap.set_local() = params.local;

		for (const auto& lparams : params.layers)
		{
			rendering::TileMapLayer layer;
			layer.tileset = context::load_tileset(lparams.tileset);

			for (const auto& tile : lparams.tiles)
				layer.paint_tile(tile);

			if (lparams.z)
				tilemap.register_layer(lparams.z.value(), std::move(layer));
			else
				tilemap.register_layer(std::move(layer));
		}

		return tilemap;
	}
}
