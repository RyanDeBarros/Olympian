#pragma once

#include "Sprites.h"

#include "../Loader.h"

namespace oly
{
	class Context;
	namespace rendering
	{
		class SpriteRegistry
		{
			std::unordered_map<std::string, std::string> texture_map;
			std::unordered_map<std::string, toml::table> sprite_constructors;
			
			std::unordered_map<std::string, std::shared_ptr<Sprite>> auto_loaded;

		public:
			void load(const Context* context, const char* sprite_registry_file);
			void load(const Context* context, const std::string& sprite_registry_file) { load(context, sprite_registry_file.c_str()); }
			void clear();

			Sprite create_sprite(const Context* context, const std::string& name) const;
			std::weak_ptr<Sprite> ref_sprite(const Context* context, const std::string& name) const;
			void delete_sprite(const Context* context, const std::string& name);
		};
	}
}
