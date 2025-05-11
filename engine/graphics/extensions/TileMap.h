#pragma once

#include "graphics/extensions/TileSet.h"

namespace oly::rendering
{
	struct TileMapLayer
	{
		TileSetRes tileset;
		
	private:
		std::unordered_map<glm::ivec2, Sprite> sprite_map;
		
	public:
		Transformer2D transformer;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void draw() const;

		void paint_tile(glm::ivec2 tile);
		void unpaint_tile(glm::ivec2 tile);

	private:
		void update_neighbour_configurations(glm::ivec2 center);
		void update_configuration(glm::ivec2 tile);
	};

	class TileMap
	{
		std::vector<TileMapLayer> layers;
			
	public:
		Transformer2D transformer;

		const TileMapLayer& layer(size_t i) const { return layers[i]; }
		TileMapLayer& layer(size_t i) { return layers[i]; }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void draw() const;

		void register_layer(TileMapLayer&& layer);
		void register_layer(size_t z, TileMapLayer&& layer);
	};
}
