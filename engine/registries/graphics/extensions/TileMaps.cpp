#include "TileMaps.h"

#include "core/context/Registries.h"
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
			toml_layers->for_each([&params](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto tileset = node["tileset"].value<std::string>();
					if (!tileset)
						return;

					params::TileMap::Layer lparams;
					lparams.tileset = tileset.value();

					auto tiles = node["tiles"].as_array();
					if (tiles)
					{
						for (const auto& toml_tile : *tiles)
						{
							if (auto _tile = toml_tile.as_array())
							{
								glm::ivec2 tile{};
								if (parse_ivec(_tile, tile))
									lparams.tiles.push_back(tile);
							}
						}
					}

					if (auto z = node["z"].value<int64_t>())
						lparams.z = (int)z.value();

					params.layers.push_back(std::move(lparams));
				}
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
