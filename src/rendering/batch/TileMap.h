#pragma once

#include "TileSet.h"

namespace oly
{
	class Context;
	namespace rendering
	{
		struct TileMapLayer
		{
			std::shared_ptr<TileSet> tileset;
		
		private:
			std::unordered_map<glm::ivec2, Sprite> sprite_map;
		
		public:
			Transformer2D transformer;

			const Transform2D& get_local() const { return transformer.get_local(); }
			Transform2D& set_local() { return transformer.set_local(); }

			void draw() const;

			void paint_tile(const Context& context, glm::ivec2 tile);
			void unpaint_tile(const Context& context, glm::ivec2 tile);

		private:
			void update_neighbour_configurations(const Context& context, glm::ivec2 center);
			void update_configuration(const Context& context, glm::ivec2 tile);
		};

		struct TileMap
		{
			std::vector<TileMapLayer> layers;
			Transformer2D transformer;

			const Transform2D& get_local() const { return transformer.get_local(); }
			Transform2D& set_local() { return transformer.set_local(); }

			void draw() const;

			void register_layer(TileMapLayer&& layer);
			void register_layer(size_t z, TileMapLayer&& layer);
		};
	}
}
