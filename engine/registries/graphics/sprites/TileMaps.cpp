#include "TileMaps.h"

#include "core/context/rendering/Tilesets.h"
#include "registries/Loader.h"

namespace oly::reg
{
	rendering::TileMap load_tilemap(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing tilemap [" << (src ? *src : "") << "]." << LOG.nl;
		}

		params::TileMap params;

		params.local = reg::load_transform_2d(node["transform"]);

		auto toml_layers = node["layer"].as_array();
		if (toml_layers)
		{
			size_t _layer_idx = 0;
			toml_layers->for_each([&params, &_layer_idx](auto&& _node) {
				const size_t layer_idx = _layer_idx++;
				TOMLNode node = (TOMLNode)_node;

				auto tileset = node["tileset"].value<std::string>();
				if (!tileset)
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse tilemap layer #" << layer_idx << " - missing \"tileset\" string field." << LOG.nl;
					return;
				}

				params::TileMap::Layer lparams;
				lparams.tileset = tileset.value();

				auto tiles = node["tiles"].as_array();
				if (tiles)
				{
					size_t tile_idx = 0;
					for (auto& toml_tile : *tiles)
					{
						glm::ivec2 tile{};
						if (parse_ivec((TOMLNode)toml_tile, tile))
							lparams.tiles.push_back(tile);
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "In tilemap layer #" << layer_idx
															<< ", cannot convert tile #" << tile_idx << " to vec2." << LOG.nl;
						++tile_idx;
					}
				}

				int z = 0;
				if (parse_int(node["z"], z))
					lparams.z = z;

				params.layers.push_back(std::move(lparams));
				});
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Tilemap [" << (src ? *src : "") << "] parsed." << LOG.nl;
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
