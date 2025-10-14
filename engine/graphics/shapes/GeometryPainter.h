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
	// TODO v6 combine ellipse and polygon shaders.
	// TODO v6 write texture in separate thread
	// The GeometryPainter class supports drawing polygons and ellipses to a texture by writing to an internal framebuffer. Use its polygon/ellipse batches to paint renderables.
	class GeometryPainter
	{
		static const int TEXTURE_CPP = 4;

		PolygonBatch polygon_batch;
		EllipseBatch ellipse_batch;
		StaticSprite sprite;
		graphics::Framebuffer framebuffer;
		graphics::BindlessTextureRef texture;
		glm::ivec2 dimensions;
		mutable bool dirty = false;

		struct WindowResizeHandler : public EventHandler<input::WindowResizeEventData>
		{
			GeometryPainter& painter;

			WindowResizeHandler(GeometryPainter& painter);

			bool consume(const input::WindowResizeEventData& data) override;
		} window_resize_handler;
		friend struct WindowResizeHandler;

	public:
		class PaintSupport
		{
			const GeometryPainter& painter;

			enum class Batch
			{
				NONE,
				POLYGON,
				ELLIPSE
			} batch = Batch::NONE;

			friend class GeometryPainter;
			PaintSupport(const GeometryPainter& painter) : painter(painter) {}

		public:
			void pre_polygon_draw();
			void pre_ellipse_draw();

		private:
			void final_flush();
		};

		using PaintFunction = std::function<void(PaintSupport&)>;

		PaintFunction paint_fn = [](PaintSupport) {};

		GeometryPainter(const PaintFunction& paint_fn);
		GeometryPainter(const PaintFunction& paint_fn, Unbatched);
		GeometryPainter(const PaintFunction& paint_fn, SpriteBatch& batch);
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

		auto get_sprite_batch() const { return sprite.get_batch(); }
		void set_sprite_batch(Unbatched) { sprite.set_batch(UNBATCHED); sync_sprite_batch(); }
		void set_sprite_batch(SpriteBatch& batch) { sprite.set_batch(batch); sync_sprite_batch(); }

	private:
		void sync_sprite_batch();

	public:
		const PolygonBatch& get_polygon_batch() const { return polygon_batch; }
		PolygonBatch& get_polygon_batch() { return polygon_batch; }
		const EllipseBatch& get_ellipse_batch() const { return ellipse_batch; }
		EllipseBatch& get_ellipse_batch() { return ellipse_batch; }

	private:
		void setup_texture();
		void copy_texture(const graphics::BindlessTexture& other);
		void sync_texture();
		void tex_image();
		void set_and_use_texture_handle();
		void setup_framebuffer();
	};
}
