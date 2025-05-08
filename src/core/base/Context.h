#pragma once

#include "core/platform/Platform.h"
#include "core/platform/WindowResize.h"

#include "registries/graphics/Textures.h"
#include "registries/graphics/primitives/SpriteRegistry.h"
#include "registries/graphics/primitives/PolygonRegistry.h"
#include "registries/graphics/primitives/EllipseRegistry.h"
#include "registries/graphics/extensions/TileSetRegistry.h"
#include "registries/graphics/extensions/TileMapRegistry.h"
#include "registries/graphics/text/FontFaceRegistry.h"
#include "registries/graphics/text/FontAtlasRegistry.h"
#include "registries/graphics/text/ParagraphRegistry.h"
#include "registries/graphics/DrawCommandRegistry.h"

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
	extern void attach_standard_window_resize(const std::function<void()>& render_frame, bool boxed = true, bool stretch = true);
	extern platform::StandardWindowResize& get_standard_window_resize();

	extern rendering::SpriteBatch& sprite_batch();
	extern rendering::PolygonBatch& polygon_batch();
	extern rendering::EllipseBatch& ellipse_batch();
	extern rendering::TextBatch& text_batch();

	extern graphics::NSVGContext& nsvg_context();
	extern reg::TextureRegistry& texture_registry();

	extern reg::SpriteRegistry& sprite_registry();
	extern reg::PolygonRegistry& polygon_registry();
	extern reg::EllipseRegistry& ellipse_registry();

	extern reg::TileSetRegistry& tileset_registry();
	extern reg::TileMapRegistry& tilemap_registry();

	extern reg::FontFaceRegistry& font_face_registry();
	extern reg::FontAtlasRegistry& font_atlas_registry();
	extern reg::ParagraphRegistry& paragraph_registry();

	extern reg::DrawCommandRegistry draw_command_registry();

	extern bool frame();

	extern graphics::BindlessTextureRes load_texture(const std::string& file, unsigned int texture_index = 0);
	extern graphics::BindlessTextureRes load_svg_texture(const std::string& file, float svg_scale = 1.0f, unsigned int texture_index = 0);
	extern glm::vec2 get_texture_dimensions(const std::string& file, unsigned int texture_index = 0);

	extern void sync_texture_handle(const graphics::BindlessTextureRes& texture);
	extern void sync_texture_handle(const graphics::BindlessTextureRes& texture, glm::vec2 dimensions);

	extern rendering::Sprite sprite();
	extern rendering::Sprite sprite(const std::string& name);
	extern std::weak_ptr<rendering::Sprite> ref_sprite(const std::string& name);
	extern void render_sprites();
	extern rendering::SpriteAtlasResExtension atlas_extension(const std::string& name);
	extern std::weak_ptr<rendering::SpriteAtlasResExtension> ref_atlas_extension(const std::string& name);

	extern rendering::Polygon polygon();
	extern rendering::Polygon polygon(const std::string& name);
	extern rendering::PolyComposite poly_composite();
	extern rendering::PolyComposite poly_composite(const std::string& name);
	extern rendering::NGon ngon();
	extern rendering::NGon ngon(const std::string& name);
	extern std::weak_ptr<rendering::Polygonal> ref_polygonal(const std::string& name);
	extern std::weak_ptr<rendering::Polygon> ref_polygon(const std::string& name);
	extern std::weak_ptr<rendering::PolyComposite> ref_poly_composite(const std::string& name);
	extern std::weak_ptr<rendering::NGon> ref_ngon(const std::string& name);
	extern void render_polygons();

	extern rendering::Ellipse ellipse();
	extern rendering::Ellipse ellipse(const std::string& name);
	extern std::weak_ptr<rendering::Ellipse> ref_ellipse(const std::string& name);
	extern void render_ellipses();

	extern rendering::TileSet tileset(const std::string& name);
	extern std::weak_ptr<rendering::TileSet> ref_tileset(const std::string& name);
	extern rendering::TileMap tilemap(const std::string& name);
	extern std::weak_ptr<rendering::TileMap> ref_tilemap(const std::string& name);
	extern rendering::FontFace font_face(const std::string& name);
	extern std::weak_ptr<rendering::FontFace> ref_font_face(const std::string& name);
	extern rendering::FontAtlas font_atlas(const std::string& name);
	extern std::weak_ptr<rendering::FontAtlas> ref_font_atlas(const std::string& name);

	extern rendering::Paragraph paragraph(const rendering::FontAtlasRes& font_atlas, const rendering::ParagraphFormat& format = {}, utf::String&& text = "");
	extern rendering::Paragraph paragraph(const std::string& name);
	extern std::weak_ptr<rendering::Paragraph> ref_paragraph(const std::string& name);
	extern void render_text();

	extern void execute_draw_command(const std::string& name);
}
