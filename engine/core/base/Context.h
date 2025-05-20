#pragma once

#include "core/platform/Platform.h"
#include "core/platform/WindowResize.h"

#include "graphics/primitives/Sprites.h"
#include "graphics/primitives/Polygons.h"
#include "graphics/primitives/Ellipses.h"
#include "graphics/text/Paragraph.h"

#include "registries/Loader.h"
#include "registries/graphics/TextureRegistry.h"
#include "registries/graphics/extensions/TileSetRegistry.h"
#include "registries/graphics/text/FontFaceRegistry.h"
#include "registries/graphics/text/FontAtlasRegistry.h"

namespace oly::context
{
	class Context
	{
	public:
		Context(const char* context_filepath);
		Context(const Context&);
		Context(Context&&) noexcept;
		~Context();
		Context& operator=(const Context&);
		Context& operator=(Context&&) noexcept;
	};

	extern const std::string& context_filepath();
	extern platform::Platform& get_platform();
	extern void set_render_function(const std::shared_ptr<Functor<void()>>& render_frame);
	extern void set_window_resized_parameters(bool boxed = true, bool stretch = true);
	extern platform::StandardWindowResize& get_standard_window_resize();

	extern rendering::SpriteBatch& sprite_batch();
	extern rendering::PolygonBatch& polygon_batch();
	extern rendering::EllipseBatch& ellipse_batch();
	extern rendering::TextBatch& text_batch();

	extern graphics::NSVGContext& nsvg_context();
	extern reg::TextureRegistry& texture_registry();
	extern reg::TileSetRegistry& tileset_registry();
	extern reg::FontFaceRegistry& font_face_registry();
	extern reg::FontAtlasRegistry& font_atlas_registry();

	extern toml::parse_result load_toml(const char* file);
	inline toml::parse_result load_toml(const std::string& file) { return load_toml(file.c_str()); }

	extern bool frame();

	extern graphics::BindlessTextureRes load_texture(const std::string& file, unsigned int texture_index = 0);
	extern graphics::BindlessTextureRes load_svg_texture(const std::string& file, float svg_scale = 1.0f, unsigned int texture_index = 0);
	extern glm::vec2 get_texture_dimensions(const std::string& file, unsigned int texture_index = 0);

	extern void sync_texture_handle(const graphics::BindlessTextureRes& texture);

	extern rendering::Sprite sprite();
	extern void render_sprites();

	extern rendering::Polygon polygon();
	extern rendering::PolyComposite poly_composite();
	extern rendering::NGon ngon();
	extern void render_polygons();

	extern rendering::Ellipse ellipse();
	extern void render_ellipses();

	extern rendering::TileSetRes load_tileset(const std::string& file);
	extern rendering::FontFaceRes load_font_face(const std::string& file);
	extern rendering::FontAtlasRes load_font_atlas(const std::string& file, unsigned int index = 0);

	extern rendering::Paragraph paragraph(const std::string& font_atlas, const rendering::ParagraphFormat& format = {}, utf::String&& text = "", unsigned int atlas_index = 0);
	extern void render_text();

	extern glm::vec2 get_cursor_screen_pos();
}
