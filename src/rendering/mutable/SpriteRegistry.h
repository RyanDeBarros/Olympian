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
			
			typedef std::vector<std::string> DrawList;
			std::unordered_map<std::string, DrawList> draw_lists;
			mutable std::unordered_map<std::string, std::shared_ptr<Sprite>> registered_sprites;

		public:
			void load(const char* sprite_registry_file);
			void load(const std::string& sprite_registry_file) { load(sprite_registry_file.c_str()); }
			void clear();

			Sprite create_sprite(const Context* context, const std::string& name) const;
			void register_sprite(const Context* context, const std::string& name) const;
			std::weak_ptr<Sprite> get_sprite(const Context* context, const std::string& name, bool register_if_nonexistant = false) const;
			void delete_sprite(const Context* context, const std::string& name) const;
			void draw_sprites(const Context* context, const std::string& draw_list_name, bool register_if_nonexistant = false) const;
		};
	}
}
