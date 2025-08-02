#pragma once

#include "external/TOML.h"
#include "graphics/backend/basic/Textures.h"
#include "graphics/text/Paragraph.h"

namespace oly::rendering
{
	class TileSet;
	class FontFace;
	class FontAtlas;
}

namespace oly::reg
{
	class TextureRegistry;
	class TileSetRegistry;
	class FontFaceRegistry;
	class FontAtlasRegistry;
}

namespace oly::context
{
	extern reg::TextureRegistry& texture_registry();
	extern reg::TileSetRegistry& tileset_registry();
	extern reg::FontFaceRegistry& font_face_registry();
	extern reg::FontAtlasRegistry& font_atlas_registry();

	extern toml::parse_result load_toml(const char* file);
	inline toml::parse_result load_toml(const std::string& file) { return load_toml(file.c_str()); }

	extern graphics::BindlessTextureRef load_texture(const std::string& file, unsigned int texture_index = 0);
	extern graphics::BindlessTextureRef load_svg_texture(const std::string& file, float svg_scale = 1.0f, unsigned int texture_index = 0);
	extern glm::vec2 get_texture_dimensions(const std::string& file, unsigned int texture_index = 0);

	extern SmartReference<rendering::TileSet> load_tileset(const std::string& file);
	extern SmartReference<rendering::FontFace> load_font_face(const std::string& file);
	extern SmartReference<rendering::FontAtlas> load_font_atlas(const std::string& file, unsigned int index = 0);

	extern rendering::Paragraph paragraph(const std::string& font_atlas, const rendering::ParagraphFormat& format = {}, utf::String&& text = "", unsigned int atlas_index = 0);
}
