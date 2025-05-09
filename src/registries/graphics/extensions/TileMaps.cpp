#include "TileMaps.h"

#include "registries/Loader.h"
#include "core/base/Context.h"

namespace oly::reg
{
	rendering::TileMap load_tilemap(const TOMLNode& node)
	{
		rendering::TileMap tilemap;

		tilemap.set_local() = reg::load_transform_2d(node, "transform");

		auto toml_layers = node["layer"].as_array();
		if (toml_layers)
		{
			toml_layers->for_each([&tilemap](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto tileset = node["tileset"].value<std::string>();
					if (!tileset)
						return;

					rendering::TileMapLayer layer;
					layer.tileset = context::ref_tileset(tileset.value()).lock();

					auto tiles = node["tiles"].as_array();
					if (tiles)
					{
						for (const auto& toml_tile : *tiles)
						{
							if (auto _tile = toml_tile.as_array())
							{
								glm::vec2 tile{};
								if (reg::parse_vec2(_tile, tile))
									layer.paint_tile({ (int)tile.x, (int)tile.y });
							}
						}
					}

					auto z = node["z"].value<int64_t>();
					if (z)
						tilemap.register_layer((size_t)z.value(), std::move(layer));
					else
						tilemap.register_layer(std::move(layer));
				}
				});
		}

		return tilemap;
	}
}
