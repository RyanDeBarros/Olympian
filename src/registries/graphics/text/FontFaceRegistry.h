#pragma once

#include "graphics/text/Font.h"

namespace oly::reg
{
	class FontFaceRegistry
	{
		struct Constructor
		{
			std::string font_file;
			rendering::Kerning kerning;
		};

		std::unordered_map<std::string, Constructor> constructors;
		std::unordered_map<std::string, rendering::FontFaceRes> auto_loaded;

	public:
		void load(const char* font_face_registry_file);
		void load(const std::string& font_face_registry_file) { load(font_face_registry_file.c_str()); }
		void clear();

		// TODO only clear(), load_font_face(file), and delete_font_face(file)

		rendering::FontFace create_font_face(const std::string& name) const;
		std::weak_ptr<rendering::FontFace> ref_font_face(const std::string& name) const;
		void delete_font_face(const std::string& name);
	};
}
