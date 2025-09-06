#include "TileMap.h"

#include "core/context/rendering/Textures.h"
#include "registries/Loader.h"
#include "registries/graphics/TextureRegistry.h"

namespace oly::rendering
{
	void TileMapLayer::draw(BatchBarrier barrier) const
	{
		auto it = sprite_map.begin();
		if (it != sprite_map.end())
			it++->second.draw(barrier);
		while (it != sprite_map.end())
			it++->second.draw(batch::PARALLEL);
	}

	void TileMapLayer::paint_tile(glm::ivec2 tile)
	{
		if (!sprite_map.count(tile))
		{
			auto& sprite = sprite_map.emplace(tile, Sprite()).first->second;
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
		
		sprite.set_local().scale = 1.0f / context::get_texture_dimensions(tile_desc.name);
	}

	void TileMap::draw(BatchBarrier barrier) const
	{
		layers[0].draw(barrier);
		for (size_t i = 1; i < layers.size(); ++i)
			layers[i].draw(batch::PARALLEL);
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
}
