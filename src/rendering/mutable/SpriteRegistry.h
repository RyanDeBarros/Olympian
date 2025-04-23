#pragma once

#include "Sprites.h"

#include "../Loader.h"

namespace oly
{
	class Context;
	namespace mut
	{
		class SpriteRegistry
		{
			std::unordered_map<std::string, std::string> texture_map;
			std::unordered_map<std::string, toml::table> sprite_constructors;

		public:
			void load(const char* sprite_registry_file);
			void load(const std::string& sprite_registry_file) { load(sprite_registry_file.c_str()); }

			Sprite create_sprite(const Context* context, const std::string& name) const;
		};
	}
}
