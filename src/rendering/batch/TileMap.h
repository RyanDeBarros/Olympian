#pragma once

#include "TileSet.h"

#include "../Loader.h"

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

			void load(const Context& context, const toml::table& node);
		};

		class TileMapRegistry
		{
			std::unordered_map<std::string, toml::table> tilemap_constructors;
			std::unordered_map<std::string, std::shared_ptr<TileMap>> auto_loaded;

		public:
			void load(const Context& context, const char* tilemap_file);
			void load(const Context& context, const std::string& tilemap_file) { load(context, tilemap_file.c_str()); }
			void clear();

			TileMap create_tilemap(const Context& context, const std::string& name) const;
			std::weak_ptr<TileMap> ref_tilemap(const std::string& name) const;
			void delete_tilemap(const std::string& name);
		};
	}
}
