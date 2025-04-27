#pragma once

#include "rendering/core/Core.h"
#include "rendering/TextureRegistry.h"
#include "rendering/batch/SpriteRegistry.h"
#include "rendering/batch/PolygonRegistry.h"
#include "rendering/batch/EllipseRegistry.h"
#include "rendering/batch/DrawCommands.h"

namespace oly
{
	class Context
	{
		struct
		{
			std::unique_ptr<rendering::Window> window;
			TextureRegistry texture_registry;
			rendering::NSVGContext nsvg_context;

			rendering::SpriteRegistry sprite_registry;
			rendering::PolygonRegistry polygon_registry;
			rendering::EllipseRegistry ellipse_registry;

			std::unique_ptr<rendering::SpriteBatch> sprite_batch;
			std::unique_ptr<rendering::PolygonBatch> polygon_batch;
			std::unique_ptr<rendering::EllipseBatch> ellipse_batch;
			
			rendering::DrawCommandRegistry draw_command_registry;
		} internal;

	public:
		Context(const char* filepath);
		Context(const Context&) = delete;
		~Context();

		GLenum per_frame_clear_mask = GL_COLOR_BUFFER_BIT;
		bool frame() const;

		const rendering::Window& window() const { return *internal.window; }
		rendering::Window& window() { return *internal.window; }
		const TextureRegistry& texture_registry() const { return internal.texture_registry; }
		TextureRegistry& texture_registry() { return internal.texture_registry; }
		const rendering::NSVGContext& nsvg_context() const { return internal.nsvg_context; }
		rendering::NSVGContext& nsvg_context() { return internal.nsvg_context; }
		void sync_texture_handle(const rendering::BindlessTextureRes& texture) const;
		void sync_texture_handle(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions) const;
		
		const rendering::SpriteRegistry& sprite_registry() const { return internal.sprite_registry; }
		rendering::SpriteRegistry& sprite_registry() { return internal.sprite_registry; }
		const rendering::PolygonRegistry& polygon_registry() const { return internal.polygon_registry; }
		rendering::PolygonRegistry& polygon_registry() { return internal.polygon_registry; }
		const rendering::EllipseRegistry& ellipse_registry() const { return internal.ellipse_registry; }
		rendering::EllipseRegistry& ellipse_registry() { return internal.ellipse_registry; }
		const rendering::DrawCommandRegistry& draw_command_registry() const { return internal.draw_command_registry; }
		rendering::DrawCommandRegistry& draw_command_registry() { return internal.draw_command_registry; }

		const rendering::SpriteBatch& sprite_batch() const { return *internal.sprite_batch; }
		rendering::SpriteBatch& sprite_batch() { return *internal.sprite_batch; }
		rendering::Sprite sprite() const { return rendering::Sprite(internal.sprite_batch.get()); }
		rendering::Sprite sprite(const std::string& name) const { return internal.sprite_registry.create_sprite(this, name); }
		std::weak_ptr<rendering::Sprite> ref_sprite(const std::string& name) const { return internal.sprite_registry.ref_sprite(this, name); }
		void render_sprites() const { internal.sprite_batch->render(); }

		const rendering::PolygonBatch& polygon_batch() const { return *internal.polygon_batch; }
		rendering::PolygonBatch& polygon_batch() { return *internal.polygon_batch; }
		rendering::Polygon polygon() const { return rendering::Polygon(internal.polygon_batch.get()); }
		rendering::Polygon polygon(const std::string& name) const { return internal.polygon_registry.create_polygon(this, name); }
		rendering::Composite composite() const { return rendering::Composite(internal.polygon_batch.get()); }
		rendering::Composite composite(const std::string& name) const { return internal.polygon_registry.create_composite(this, name); }
		rendering::NGon ngon() const { return rendering::NGon(internal.polygon_batch.get()); }
		rendering::NGon ngon(const std::string& name) const { return internal.polygon_registry.create_ngon(this, name); }
		std::weak_ptr<rendering::Polygonal> ref_polygonal(const std::string& name) const { return internal.polygon_registry.ref_polygonal(name); }
		std::weak_ptr<rendering::Polygon> ref_polygon(const std::string& name) const { return std::dynamic_pointer_cast<rendering::Polygon>(ref_polygonal(name).lock()); }
		std::weak_ptr<rendering::Composite> ref_composite(const std::string& name) const { return std::dynamic_pointer_cast<rendering::Composite>(ref_polygonal(name).lock()); }
		std::weak_ptr<rendering::NGon> ref_ngon(const std::string& name) const { return std::dynamic_pointer_cast<rendering::NGon>(ref_polygonal(name).lock()); }
		void render_polygons() const { internal.polygon_batch->render(); }

		const rendering::EllipseBatch& ellipse_batch() const { return *internal.ellipse_batch; }
		rendering::EllipseBatch& ellipse_batch() { return *internal.ellipse_batch; }
		rendering::Ellipse ellipse() const { return rendering::Ellipse(internal.ellipse_batch.get()); }
		rendering::Ellipse ellipse(const std::string& name) const { return internal.ellipse_registry.create_ellipse(this, name); }
		std::weak_ptr<rendering::Ellipse> ref_ellipse(const std::string& name) const { return internal.ellipse_registry.ref_ellipse(name); }
		void render_ellipses() const { internal.ellipse_batch->render(); }

		void execute_draw_command(const std::string& name) const { internal.draw_command_registry.execute(name); }
	};
}
