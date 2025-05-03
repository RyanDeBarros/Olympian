#include "TileMap.h"

#include "Context.h"

namespace oly
{
	namespace rendering
	{
		void TileMapLayer::draw() const
		{
			for (const auto& [tile, sprite] : sprite_map)
				sprite.draw();
		}

		void TileMapLayer::paint_tile(glm::ivec2 tile)
		{
			if (!sprite_map.count(tile))
			{
				auto& sprite = sprite_map.emplace(tile, context::sprite()).first->second;
				sprite.transformer.attach_parent(&transformer);
				update_configuration(tile);
				update_neighbour_configurations(tile);
			}
		}

		void TileMapLayer::unpaint_tile(glm::ivec2 tile)
		{
			auto it = sprite_map.find(tile);
			if (it != sprite_map.end())
			{
				sprite_map.erase(it);
				update_neighbour_configurations(tile);
			}
		}

		void TileMapLayer::update_neighbour_configurations(glm::ivec2 center)
		{
			update_configuration(center + glm::ivec2{  1,  0 });
			update_configuration(center + glm::ivec2{  0,  1 });
			update_configuration(center + glm::ivec2{ -1,  0 });
			update_configuration(center + glm::ivec2{  0, -1 });
			update_configuration(center + glm::ivec2{  1,  1 });
			update_configuration(center + glm::ivec2{ -1,  1 });
			update_configuration(center + glm::ivec2{ -1, -1 });
			update_configuration(center + glm::ivec2{  1, -1 });
		}

		void TileMapLayer::update_configuration(glm::ivec2 tile)
		{
			auto it = sprite_map.find(tile);
			if (it == sprite_map.end())
				return;
			Sprite& sprite = it->second;

			TileSet::PaintedTile painted_tile;
			painted_tile.orthogonal[0] = sprite_map.count(tile + glm::ivec2{  1,  0 });
			painted_tile.orthogonal[1] = sprite_map.count(tile + glm::ivec2{  0,  1 });
			painted_tile.orthogonal[2] = sprite_map.count(tile + glm::ivec2{ -1,  0 });
			painted_tile.orthogonal[3] = sprite_map.count(tile + glm::ivec2{  0, -1 });
			painted_tile.diagonal[0]   = sprite_map.count(tile + glm::ivec2{  1,  1 });
			painted_tile.diagonal[1]   = sprite_map.count(tile + glm::ivec2{ -1,  1 });
			painted_tile.diagonal[2]   = sprite_map.count(tile + glm::ivec2{ -1, -1 });
			painted_tile.diagonal[3]   = sprite_map.count(tile + glm::ivec2{  1, -1 });
			
			TileSet::Transformation transformation;
			TileSet::TileDesc tile_desc = tileset->get_tile_desc(painted_tile, transformation);

			sprite.set_texture(tile_desc.name);
			sprite.set_tex_coords(tile_desc.uvs);
			sprite.set_local().position = glm::vec2(tile);
			if (transformation & TileSet::Transformation::REFLECT_X)
				sprite.set_local().scale.x = -glm::abs(sprite.set_local().scale.x);
			else
				sprite.set_local().scale.x = glm::abs(sprite.set_local().scale.x);
			if (transformation & TileSet::Transformation::REFLECT_Y)
				sprite.set_local().scale.y = -glm::abs(sprite.set_local().scale.y);
			else
				sprite.set_local().scale.y = glm::abs(sprite.set_local().scale.y);
			if (transformation & TileSet::Transformation::ROTATE_90)
				sprite.set_local().rotation = glm::radians(90.0f);
			else if (transformation & TileSet::Transformation::ROTATE_180)
				sprite.set_local().rotation = glm::radians(180.0f);
			else if (transformation & TileSet::Transformation::ROTATE_270)
				sprite.set_local().rotation = glm::radians(270.0f);
			else
				sprite.set_local().rotation = 0.0f;
			
			auto idim = context::texture_registry().get_image_dimensions(tile_desc.name);
			sprite.set_local().scale = { 1.0f / idim.w, 1.0f / idim.h };
		}

		void TileMap::draw() const
		{
			for (const auto& layer : layers)
				layer.draw();
		}

		void TileMap::register_layer(TileMapLayer&& layer)
		{
			layer.transformer.attach_parent(&transformer);
			layers.push_back(std::move(layer));
		}
		
		void TileMap::register_layer(size_t z, TileMapLayer&& layer)
		{
			layer.transformer.attach_parent(&transformer);
			layers.insert(layers.begin() + z, std::move(layer));
		}

		void TileMap::load(const toml::table& node)
		{
			set_local() = assets::load_transform_2d(node, "transform");

			auto toml_layers = node["layer"].as_array();
			if (toml_layers)
			{
				toml_layers->for_each([this](auto&& node) {
					if constexpr (toml::is_table<decltype(node)>)
					{
						auto tileset = node["tileset"].value<std::string>();
						if (!tileset)
							return;

						TileMapLayer layer;
						layer.tileset = context::ref_tileset(tileset.value()).lock();
						
						auto tiles = node["tiles"].as_array();
						if (tiles)
						{
							for (const auto& toml_tile : *tiles)
							{
								if (auto _tile = toml_tile.as_array())
								{
									glm::vec2 tile{};
									if (assets::parse_vec2(_tile, tile))
										layer.paint_tile({ (int)tile.x, (int)tile.y });
								}
							}
						}

						auto z = node["z"].value<int64_t>();
						if (z)
							register_layer((size_t)z.value(), std::move(layer));
						else
							register_layer(std::move(layer));
					}
					});
			}
		}

		void TileMapRegistry::load(const char* tilemap_file)
		{
			auto toml = assets::load_toml(tilemap_file);
			auto tilemap_list = toml["tilemap"].as_array();
			if (!tilemap_list)
				return;
			tilemap_list->for_each([this](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					if (auto _name = node["name"].value<std::string>())
					{
						const std::string& name = _name.value();
						tilemap_constructors[name] = node;
						if (auto _init = node["init"].value<std::string>())
						{
							auto_loaded.emplace(name, std::make_shared<TileMap>(create_tilemap(name)));
							if (_init.value() == "discard")
								tilemap_constructors.erase(name);
						}
					}
				}
				});
		}
		
		void TileMapRegistry::clear()
		{
			tilemap_constructors.clear();
			auto_loaded.clear();
		}
		
		TileMap TileMapRegistry::create_tilemap(const std::string& name) const
		{
			auto it = tilemap_constructors.find(name);
			if (it == tilemap_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_TILEMAP);
			const auto& node = it->second;

			TileMap tilemap;
			tilemap.load(node);
			return tilemap;
		}
		
		std::weak_ptr<TileMap> TileMapRegistry::ref_tilemap(const std::string& name) const
		{
			auto it = auto_loaded.find(name);
			if (it == auto_loaded.end())
				throw Error(ErrorCode::UNREGISTERED_TILEMAP);
			return it->second;
		}
		
		void TileMapRegistry::delete_tilemap(const std::string& name)
		{
			auto_loaded.erase(name);
		}
	}
}
