#pragma once

#include "graphics/sprites/Sprite.h"
#include "graphics/shapes/Ellipses.h"
#include "graphics/shapes/Polygons.h"
#include "graphics/backend/basic/Framebuffers.h"

#include "core/context/rendering/Scopes.h"

namespace oly::rendering
{
	// TODO v6 update GeometryPainter mkdocs after modifying
	// TODO v6 combine ellipse and polygon shaders?

	class GeometryPainter
	{
		PolygonBatch polygon_batch;
		EllipseBatch ellipse_batch;
		graphics::Framebuffer framebuffer;

	public:
		GeometryPainter() = default;
		GeometryPainter(const GeometryPainter&);
		GeometryPainter(GeometryPainter&&) noexcept;
		GeometryPainter& operator=(const GeometryPainter&);
		GeometryPainter& operator=(GeometryPainter&&) noexcept;

		const PolygonBatch& get_polygon_batch() const { return polygon_batch; }
		PolygonBatch& get_polygon_batch() { return polygon_batch; }
		const EllipseBatch& get_ellipse_batch() const { return ellipse_batch; }
		EllipseBatch& get_ellipse_batch() { return ellipse_batch; }

		class PaintContext
		{
			GeometryPainter& painter;
			context::ScopedViewportChange scope;
			graphics::BindlessTextureRef texture;
			glm::ivec2 dimensions;

			enum class Batch
			{
				NONE,
				POLYGON,
				ELLIPSE
			} batch = Batch::NONE;

			friend class GeometryPainter;
			PaintContext(GeometryPainter& painter, const Camera2DRef& camera, math::IRect2D bounds, float rotation, int texture_cpp);
			PaintContext(const PaintContext&) = delete;
			PaintContext(PaintContext&&) = delete;
		
		public:
			~PaintContext();

			void pre_polygon_draw();
			void pre_ellipse_draw();
			void flush();

			glm::vec2 get_texture_dimensions() const { return dimensions; }
			graphics::BindlessTextureRef get_texture() const { return texture; }

			void set_texture(StaticSprite& sprite) const;
			void set_texture(Sprite& sprite) const;
		};

		PaintContext paint_context(const Camera2DRef& camera, math::IRect2D bounds, float rotation = 0.0f, int texture_cpp = 4);

	private:
		mutable bool context_locked = false;
	};
}
