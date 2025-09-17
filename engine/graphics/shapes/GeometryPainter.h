#pragma once

#include "graphics/sprites/Sprite.h"
#include "graphics/shapes/Ellipses.h"
#include "graphics/shapes/Polygons.h"
#include "graphics/backend/basic/Framebuffers.h"

#include "core/platform/WindowEvents.h"
#include "core/platform/EventHandler.h"

#include <functional>

namespace oly::rendering
{
	// TODO v4 add to mkdocs
	// TODO v4 combine ellipse and polygon shaders.
	// TODO v5 write texture in separate thread
	// The GeometryPainter class supports drawing polygons and ellipses to a texture by writing to an internal framebuffer. Use its polygon/ellipse batches to paint renderables.
	class GeometryPainter
	{
		static const int TEXTURE_CPP = 4;

		rendering::PolygonBatch polygon_batch;
		rendering::EllipseBatch ellipse_batch;
		rendering::StaticSprite sprite;
		graphics::Framebuffer framebuffer;
		graphics::BindlessTextureRef texture;
		glm::ivec2 dimensions;
		mutable bool dirty = false;

		struct WindowResizeHandler : public EventHandler<input::WindowResizeEventData>
		{
			GeometryPainter* painter = nullptr;

			WindowResizeHandler(GeometryPainter* painter);

			bool consume(const input::WindowResizeEventData& data) override;

			void set_projection();
		} window_resize_handler;
		friend struct WindowResizeHandler;

	public:
		std::function<void()> paint_fn = []() {};

		GeometryPainter(const std::function<void()>& paint_fn, rendering::SpriteBatch* batch = nullptr);
		GeometryPainter(const GeometryPainter&);
		GeometryPainter(GeometryPainter&&) noexcept;
		~GeometryPainter();
		GeometryPainter& operator=(const GeometryPainter&);
		GeometryPainter& operator=(GeometryPainter&&) noexcept;

	private:
		void write_texture() const;
		void set_sprite_scale(glm::vec2 scale);

	public:
		void draw() const;
		void flag_dirty() const { dirty = true; }
		void regen_to_current_resolution();

		const rendering::PolygonBatch& get_polygon_batch() const { return polygon_batch; }
		rendering::PolygonBatch& get_polygon_batch() { return polygon_batch; }
		const rendering::EllipseBatch& get_ellipse_batch() const { return ellipse_batch; }
		rendering::EllipseBatch& get_ellipse_batch() { return ellipse_batch; }

	private:
		void setup_texture();
		void copy_texture(const graphics::BindlessTexture& other);
		void sync_texture();
		void tex_image();
		void set_and_use_texture_handle();
		void setup_framebuffer();
	};
}
