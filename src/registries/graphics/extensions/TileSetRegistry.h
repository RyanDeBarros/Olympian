#pragma once

#include "graphics/extensions/TileSet.h"

namespace oly::reg
{
	class TileSetRegistry
	{
		std::unordered_map<std::string, toml::array> tileset_constructors;
		std::unordered_map<std::string, std::shared_ptr<rendering::TileSet>> auto_loaded;

	public:
		void load(const char* tileset_file);
		void load(const std::string& tileset_file) { load(tileset_file.c_str()); }
		void clear();

		rendering::TileSet create_tileset(const std::string& name) const;
		std::weak_ptr<rendering::TileSet> ref_tileset(const std::string& name) const;
		void delete_tileset(const std::string& name);
	};
}
