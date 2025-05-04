#pragma once

#include "graphics/primitives/Sprites.h"
#include "registries/Loader.h"

namespace oly
{
	namespace rendering
	{
		class SpriteRegistry
		{
			std::unordered_map<std::string, std::string> texture_map;
			std::unordered_map<std::string, toml::table> sprite_constructors;
			
			std::unordered_map<std::string, std::shared_ptr<Sprite>> auto_loaded;

			std::unordered_map<std::string, toml::table> atlas_constructors;
			std::unordered_map<std::string, std::shared_ptr<AtlasResExtension>> auto_loaded_atlas_extensions;

		public:
			void load(const char* sprite_registry_file);
			void load(const std::string& sprite_registry_file) { load(sprite_registry_file.c_str()); }
			void clear();

			Sprite create_sprite(const std::string& name) const;
			std::weak_ptr<Sprite> ref_sprite(const std::string& name) const;
			void delete_sprite(const std::string& name);

			AtlasResExtension create_atlas_extension(const std::string& name) const;
			std::weak_ptr<AtlasResExtension> ref_atlas_extension(const std::string& name) const;
			void delete_atlas_extension(const std::string& name);
		};
	}
}
