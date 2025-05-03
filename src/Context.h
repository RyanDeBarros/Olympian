#pragma once

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

		extern rendering::SpriteBatch& sprite_batch();
		extern rendering::PolygonBatch& polygon_batch();
		extern rendering::EllipseBatch& ellipse_batch();

		extern void sync_texture_handle(const rendering::BindlessTextureRes& texture);
		extern void sync_texture_handle(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions);

		// TODO make all of these inline functions extern
		inline rendering::Sprite sprite() { return rendering::Sprite(sprite_batch()); }
		inline rendering::Sprite sprite(const std::string& name) { return context::sprite_registry().create_sprite(name); }
		inline std::weak_ptr<rendering::Sprite> ref_sprite(const std::string& name) { return context::sprite_registry().ref_sprite(name); }
		inline void render_sprites() { sprite_batch().render(); }
		inline rendering::AtlasResExtension atlas_extension(const std::string& name) { return context::sprite_registry().create_atlas_extension(name); }
		inline std::weak_ptr<rendering::AtlasResExtension> ref_atlas_extension(const std::string& name) { return context::sprite_registry().ref_atlas_extension(name); }

		inline rendering::Polygon polygon() { return rendering::Polygon(polygon_batch()); }
		inline rendering::Polygon polygon(const std::string& name) { return context::polygon_registry().create_polygon(name); }
		inline rendering::Composite composite() { return rendering::Composite(polygon_batch()); }
		inline rendering::Composite composite(const std::string& name) { return context::polygon_registry().create_composite(name); }
		inline rendering::NGon ngon() { return rendering::NGon(polygon_batch()); }
		inline rendering::NGon ngon(const std::string& name) { return context::polygon_registry().create_ngon(name); }
		inline std::weak_ptr<rendering::Polygonal> ref_polygonal(const std::string& name) { return context::polygon_registry().ref_polygonal(name); }
		inline std::weak_ptr<rendering::Polygon> ref_polygon(const std::string& name) { return std::dynamic_pointer_cast<rendering::Polygon>(ref_polygonal(name).lock()); }
		inline std::weak_ptr<rendering::Composite> ref_composite(const std::string& name) { return std::dynamic_pointer_cast<rendering::Composite>(ref_polygonal(name).lock()); }
		inline std::weak_ptr<rendering::NGon> ref_ngon(const std::string& name) { return std::dynamic_pointer_cast<rendering::NGon>(ref_polygonal(name).lock()); }
		inline void render_polygons() { polygon_batch().render(); }

		inline rendering::Ellipse ellipse() { return rendering::Ellipse(ellipse_batch()); }
		inline rendering::Ellipse ellipse(const std::string& name) { return context::ellipse_registry().create_ellipse(name); }
		inline std::weak_ptr<rendering::Ellipse> ref_ellipse(const std::string& name) { return context::ellipse_registry().ref_ellipse(name); }
		inline void render_ellipses() { ellipse_batch().render(); }

		inline rendering::TileSet tileset(const std::string& name) { return context::tileset_registry().create_tileset(name); }
		inline std::weak_ptr<rendering::TileSet> ref_tileset(const std::string& name) { return context::tileset_registry().ref_tileset(name); }

		inline rendering::TileMap tilemap(const std::string& name) { return context::tilemap_registry().create_tilemap(name); }
		inline std::weak_ptr<rendering::TileMap> ref_tilemap(const std::string& name) { return context::tilemap_registry().ref_tilemap(name); }

		inline rendering::FontFace font_face(const std::string& name) { return context::font_face_registry().create_font_face(name); }
		inline std::weak_ptr<rendering::FontFace> ref_font_face(const std::string& name) { return context::font_face_registry().ref_font_face(name); }

		inline rendering::FontAtlas font_atlas(const std::string& name) { return context::font_atlas_registry().create_font_atlas(context::font_face_registry(), name); }
		inline std::weak_ptr<rendering::FontAtlas> ref_font_atlas(const std::string& name) { return context::font_atlas_registry().ref_font_atlas(name); }

		inline void execute_draw_command(const std::string& name) { context::draw_command_registry().execute(name); }
	}

	// TODO Context is getting large. Use static global variables in Context.cpp instead of class.
	class Context
	{
		struct
		{
			std::unique_ptr<rendering::Window> window;
			std::array<std::unique_ptr<input::Gamepad>, GLFW_JOYSTICK_LAST> gamepads;
			int num_gamepads = 0;
			input::SignalTable signal_table;
			input::BindingContext binding_context;
		} internal;

	public:
		Context(const char* filepath);
		Context(const Context&) = delete;
		~Context();

	private:
		void init_window(const assets::AssetNode& node);
		void init_gamepads(const assets::AssetNode& node);
		void init_binding_context();

	public:
		GLenum per_frame_clear_mask = GL_COLOR_BUFFER_BIT;
		bool frame();

		const rendering::Window& window() const { return *internal.window; }
		rendering::Window& window() { return *internal.window; }
		const input::Gamepad& gamepad(int i = 0) const { return *internal.gamepads[i]; }
		input::Gamepad& gamepad(int i = 0) { return *internal.gamepads[i]; }
		const input::SignalTable& signal_table() const { return internal.signal_table; }
		input::SignalTable& signal_table() { return internal.signal_table; }
		const input::BindingContext& binding_context() const { return internal.binding_context; }
		input::BindingContext& binding_context() { return internal.binding_context; }
		template<std::derived_from<input::InputController> Controller, input::GenericSignal Signal>
		void bind_signal(const char* signal, bool(Controller::* handler)(Signal), Controller& controller)
		{
			binding_context().bind(signal_table().get(signal), static_cast<input::InputController::Handler<Signal>>(handler), &controller);
		}
		template<std::derived_from<input::InputController> Controller, input::GenericSignal Signal>
		void unbind_signal(const char* signal, bool(Controller::* handler)(Signal), Controller& controller)
		{
			binding_context().unbind(signal_table().get(signal), static_cast<input::InputController::Handler<Signal>>(handler), &controller);
		}
	};
}
