#pragma once

#include "Platform.h"
#include "rendering/core/Core.h"
#include "rendering/TextureRegistry.h"
#include "rendering/batch/SpriteRegistry.h"
#include "rendering/batch/PolygonRegistry.h"
#include "rendering/batch/EllipseRegistry.h"
#include "rendering/batch/TileMap.h"
#include "rendering/batch/text/Font.h"
#include "rendering/batch/DrawCommands.h"

namespace oly
{
	namespace context
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

		extern Platform& platform();

		extern rendering::SpriteBatch& sprite_batch();
		extern rendering::PolygonBatch& polygon_batch();
		extern rendering::EllipseBatch& ellipse_batch();

		extern TextureRegistry& texture_registry();
		extern rendering::NSVGContext& nsvg_context();

		extern rendering::SpriteRegistry& sprite_registry();
		extern rendering::PolygonRegistry& polygon_registry();
		extern rendering::EllipseRegistry& ellipse_registry();

		extern rendering::TileSetRegistry& tileset_registry();
		extern rendering::TileMapRegistry& tilemap_registry();
		extern rendering::FontFaceRegistry& font_face_registry();
		extern rendering::FontAtlasRegistry& font_atlas_registry();

		extern rendering::DrawCommandRegistry draw_command_registry();

		extern bool frame();

		extern void sync_texture_handle(const rendering::BindlessTextureRes& texture);
		extern void sync_texture_handle(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions);

		extern rendering::Sprite sprite();
		extern rendering::Sprite sprite(const std::string& name);
		extern std::weak_ptr<rendering::Sprite> ref_sprite(const std::string& name);
		extern void render_sprites();
		extern rendering::AtlasResExtension atlas_extension(const std::string& name);
		extern std::weak_ptr<rendering::AtlasResExtension> ref_atlas_extension(const std::string& name);

		extern rendering::Polygon polygon();
		extern rendering::Polygon polygon(const std::string& name);
		extern rendering::Composite composite();
		extern rendering::Composite composite(const std::string& name);
		extern rendering::NGon ngon();
		extern rendering::NGon ngon(const std::string& name);
		extern std::weak_ptr<rendering::Polygonal> ref_polygonal(const std::string& name);
		extern std::weak_ptr<rendering::Polygon> ref_polygon(const std::string& name);
		extern std::weak_ptr<rendering::Composite> ref_composite(const std::string& name);
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

		extern void execute_draw_command(const std::string& name);
	}
}
