#pragma once

#include "external/TOML.h"
#include "graphics/primitives/Sprites.h"
#include "graphics/extensions/SpriteAtlas.h"

namespace oly::reg
{
	class SpriteRegistry
	{
		std::unordered_map<std::string, toml::table> sprite_constructors;		
		std::unordered_map<std::string, std::shared_ptr<rendering::Sprite>> auto_loaded_sprites;

		std::unordered_map<std::string, toml::table> sprite_atlas_constructors;
		std::unordered_map<std::string, std::shared_ptr<rendering::SpriteAtlasResExtension>> auto_loaded_sprite_atlases;

	public:
		void load(const char* sprite_registry_file);
		void load(const std::string& sprite_registry_file) { load(sprite_registry_file.c_str()); }
		void clear();

		rendering::Sprite create_sprite(const std::string& name) const;
		std::weak_ptr<rendering::Sprite> ref_sprite(const std::string& name) const;
		void delete_sprite(const std::string& name);

		rendering::SpriteAtlasResExtension create_atlas_extension(const std::string& name) const;
		std::weak_ptr<rendering::SpriteAtlasResExtension> ref_atlas_extension(const std::string& name) const;
		void delete_atlas_extension(const std::string& name);
	};
}
