#pragma once

#include "graphics/extensions/TileSet.h"
#include "core/base/TransformerExposure.h"

namespace oly::rendering
{
	struct TileMapLayer
	{
		TileSetRef tileset;
		
	private:
		std::unordered_map<glm::ivec2, Sprite> sprite_map;
		
	public:
		Transformer2D transformer;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void draw(BatchBarrier barrier = batch::BARRIER) const;

		void paint_tile(glm::ivec2 tile);
		void unpaint_tile(glm::ivec2 tile);

	private:
		void update_neighbour_configurations(glm::ivec2 center);
		void update_configuration(glm::ivec2 tile);
	};

	class TileMap
	{
		std::vector<TileMapLayer> layers;
			
		Transformer2D transformer;

	public:
		const TileMapLayer& layer(size_t i) const { return layers[i]; }
		TileMapLayer& layer(size_t i) { return layers[i]; }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<TExposureParams{ .local = exposure::local::FULL, .chain = exposure::chain::ATTACH_ONLY, .modifier = exposure::modifier::FULL }>
			set_transformer() { return transformer; }

		void draw(BatchBarrier barrier = batch::BARRIER) const;

		void register_layer(TileMapLayer&& layer);
		void register_layer(size_t z, TileMapLayer&& layer);
	};

	typedef SmartReference<TileMap> TileMapRef;
}
