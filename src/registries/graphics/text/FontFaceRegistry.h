#pragma once

#include "graphics/text/Font.h"

namespace oly
{
	namespace rendering
	{
		class FontFaceRegistry
		{
			struct Constructor
			{
				std::string font_file;
				Kerning kerning;
			};

			std::unordered_map<std::string, Constructor> constructors;
			std::unordered_map<std::string, FontFaceRes> auto_loaded;

		public:
			void load(const char* font_face_registry_file);
			void load(const std::string& font_face_registry_file) { load(font_face_registry_file.c_str()); }
			void clear();

			FontFace create_font_face(const std::string& name) const;
			std::weak_ptr<FontFace> ref_font_face(const std::string& name) const;
			void delete_font_face(const std::string& name);
		};
	}
}
