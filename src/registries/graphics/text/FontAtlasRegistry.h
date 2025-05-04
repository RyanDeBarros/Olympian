#pragma once

#include "graphics/text/Font.h"

namespace oly::reg
{
	class FontAtlasRegistry
	{
		struct Constructor
		{
			std::string font_face_name;
			rendering::FontOptions options;
			utf::String common_buffer;
		};

		std::unordered_map<std::string, Constructor> constructors;
		std::unordered_map<std::string, rendering::FontAtlasRes> auto_loaded;

	public:
		void load(const char* font_atlas_registry_file);
		void load(const std::string& font_atlas_registry_file) { load(font_atlas_registry_file.c_str()); }
		void clear();

		rendering::FontAtlas create_font_atlas(const std::string& name) const;
		std::weak_ptr<rendering::FontAtlas> ref_font_atlas(const std::string& name) const;
		void delete_font_atlas(const std::string& name);
	};
}
