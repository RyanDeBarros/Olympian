#pragma once

#include "graphics/extensions/TileMap.h"

#include "registries/Loader.h"

namespace oly
{
	namespace rendering
	{
		class TileMapRegistry
		{
			std::unordered_map<std::string, toml::table> tilemap_constructors;
			std::unordered_map<std::string, std::shared_ptr<TileMap>> auto_loaded;

		public:
			void load(const char* tilemap_file);
			void load(const std::string& tilemap_file) { load(tilemap_file.c_str()); }
			void clear();

			TileMap create_tilemap(const std::string& name) const;
			std::weak_ptr<TileMap> ref_tilemap(const std::string& name) const;
			void delete_tilemap(const std::string& name);
		};
	}
}
