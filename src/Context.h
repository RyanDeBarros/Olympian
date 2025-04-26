#pragma once

#include "rendering/core/Core.h"
#include "rendering/TextureRegistry.h"
#include "rendering/batch/SpriteRegistry.h"
#include "rendering/batch/PolygonRegistry.h"
#include "rendering/batch/EllipseRegistry.h"

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

		const rendering::SpriteBatch& sprite_batch() const { return *internal.sprite_batch; }
		rendering::SpriteBatch& sprite_batch() { return *internal.sprite_batch; }
		rendering::Sprite sprite() const { return rendering::Sprite(internal.sprite_batch.get()); }
		rendering::Sprite sprite(const std::string& name) const { return internal.sprite_registry.create_sprite(this, name); }
		std::weak_ptr<rendering::Sprite> ref_sprite(const std::string& name, bool register_if_nonexistant = true) const { return internal.sprite_registry.get_sprite(this, name, register_if_nonexistant); }
		void render_sprites() const { internal.sprite_batch->render(); }
		void draw_sprite_list(const std::string& draw_list_name, bool register_if_nonexistant = true) const { internal.sprite_registry.draw_sprites(this, draw_list_name, register_if_nonexistant); render_sprites(); }

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
		void draw_polygons(size_t draw_spec = 0) const { internal.polygon_batch->draw(draw_spec); }

		const rendering::EllipseBatch& ellipse_batch() const { return *internal.ellipse_batch; }
		rendering::EllipseBatch& ellipse_batch() { return *internal.ellipse_batch; }
		rendering::Ellipse ellipse() const { return rendering::Ellipse(internal.ellipse_batch.get()); }
		rendering::Ellipse ellipse(const std::string& name) const { return internal.ellipse_registry.create_ellipse(this, name); }
		std::weak_ptr<rendering::Ellipse> ref_ellipse(const std::string& name) const { return internal.ellipse_registry.ref_ellipse(name); }
		void draw_ellipses(size_t draw_spec = 0) const { internal.ellipse_batch->draw(draw_spec); }
	};
}
