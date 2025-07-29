#pragma once

#include "core/base/SimpleMath.h"
#include "core/platform/Platform.h"
#include "core/platform/WindowResize.h"

#include "graphics/primitives/Sprites.h"
#include "graphics/primitives/Polygons.h"
#include "graphics/primitives/Ellipses.h"
#include "graphics/text/Paragraph.h"

namespace oly
{
	namespace col2d
	{
		class CollisionDispatcher;
	}

	namespace graphics
	{
		class NSVGContext;
	}

	namespace rendering
	{
		class TileSet;
	}

	namespace reg
	{
		class TextureRegistry;
		class TileSetRegistry;
		class FontFaceRegistry;
		class FontAtlasRegistry;
	}
}

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
	extern void set_window_resize_mode(bool boxed = true, bool stretch = true);
	extern const platform::WRViewport& get_wr_viewport();
	extern platform::WRDrawer& get_wr_drawer();
	extern void set_standard_viewport();

	extern rendering::SpriteBatch& sprite_batch();
	extern rendering::PolygonBatch& polygon_batch();
	extern rendering::EllipseBatch& ellipse_batch();
	extern rendering::TextBatch& text_batch();

	enum class InternalBatch
	{
		NONE,
		SPRITE,
		POLYGON,
		ELLIPSE,
		TEXT
	};
	extern InternalBatch last_internal_batch_rendered();

	extern graphics::NSVGContext& nsvg_context();
	extern reg::TextureRegistry& texture_registry();
	extern reg::TileSetRegistry& tileset_registry();
	extern reg::FontFaceRegistry& font_face_registry();
	extern reg::FontAtlasRegistry& font_atlas_registry();

	extern toml::parse_result load_toml(const char* file);
	inline toml::parse_result load_toml(const std::string& file) { return load_toml(file.c_str()); }

	extern bool frame();
	extern BigSize this_frame();

	extern graphics::BindlessTextureRef load_texture(const std::string& file, unsigned int texture_index = 0);
	extern graphics::BindlessTextureRef load_svg_texture(const std::string& file, float svg_scale = 1.0f, unsigned int texture_index = 0);
	extern glm::vec2 get_texture_dimensions(const std::string& file, unsigned int texture_index = 0);

	extern void sync_texture_handle(const graphics::BindlessTextureRef& texture);

	extern void render_sprites();
	extern void render_polygons();
	extern void render_ellipses();

	extern SmartReference<rendering::TileSet> load_tileset(const std::string& file);
	extern SmartReference<rendering::FontFace> load_font_face(const std::string& file);
	extern SmartReference<rendering::FontAtlas> load_font_atlas(const std::string& file, unsigned int index = 0);

	extern rendering::Paragraph paragraph(const std::string& font_atlas, const rendering::ParagraphFormat& format = {}, utf::String&& text = "", unsigned int atlas_index = 0);
	extern void render_text();

	extern glm::vec2 get_cursor_screen_pos();
	extern glm::vec2 get_initial_window_size();
	extern glm::vec2 get_view_stretch();
	extern glm::vec2 get_cursor_view_pos();

	extern bool blend_enabled();
	extern glm::vec4 clear_color();

	extern col2d::CollisionDispatcher& collision_dispatcher();
}
